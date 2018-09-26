/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ametsuchi/impl/postgres_command_executor.hpp"

#include <soci/postgresql/soci-postgresql.h>
#include <boost/format.hpp>
#include "ametsuchi/impl/soci_utils.hpp"
#include "backend/protobuf/permissions.hpp"
#include "cryptography/public_key.hpp"
#include "interfaces/commands/add_asset_quantity.hpp"
#include "interfaces/commands/add_peer.hpp"
#include "interfaces/commands/add_signatory.hpp"
#include "interfaces/commands/append_role.hpp"
#include "interfaces/commands/create_account.hpp"
#include "interfaces/commands/create_asset.hpp"
#include "interfaces/commands/create_domain.hpp"
#include "interfaces/commands/create_role.hpp"
#include "interfaces/commands/detach_role.hpp"
#include "interfaces/commands/grant_permission.hpp"
#include "interfaces/commands/remove_signatory.hpp"
#include "interfaces/commands/revoke_permission.hpp"
#include "interfaces/commands/set_account_detail.hpp"
#include "interfaces/commands/set_quorum.hpp"
#include "interfaces/commands/subtract_asset_quantity.hpp"
#include "interfaces/commands/transfer_asset.hpp"
#include "interfaces/common_objects/types.hpp"

namespace {
  struct PreparedStatement {
    std::string command_name;
    std::string command_base;
    std::vector<std::string> permission_checks;

    static const std::string validationPrefix;
    static const std::string noValidationPrefix;
  };

  const std::string PreparedStatement::validationPrefix = "WithValidation";
  const std::string PreparedStatement::noValidationPrefix = "WithOutValidation";

  // Transforms prepared statement into two strings:
  //    1. SQL query with validation
  //    2. SQL query without validation
  std::pair<std::string, std::string> compileStatement(
      const PreparedStatement &statement) {
    // Create query with validation
    auto with_validation = boost::format(statement.command_base)
        % (statement.command_name + PreparedStatement::validationPrefix);

    // append all necessary checks to the query
    for (const auto &check : statement.permission_checks) {
      with_validation = with_validation % check;
    }

    // Create query without validation
    auto without_validation = boost::format(statement.command_base)
        % (statement.command_name + PreparedStatement::noValidationPrefix);

    // since checks are not needed, append empty strings to their place
    for (size_t i = 0; i < statement.permission_checks.size(); i++) {
      without_validation = without_validation % "";
    }

    return {with_validation.str(), without_validation.str()};
  }

  void prepareStatement(soci::session &sql,
                        const PreparedStatement &statement) {
    auto queries = compileStatement(statement);

    sql << queries.first;
    sql << queries.second;
  }

  iroha::expected::Error<iroha::ametsuchi::CommandError> makeCommandError(
      const std::string &error_message,
      const std::string &command_name) noexcept {
    return iroha::expected::makeError(
        iroha::ametsuchi::CommandError{command_name, error_message});
  }

  /**
   * Executes sql query
   * Assumes that statement query returns 0 in case of success
   * or error code in case of failure
   * @param sql - connection on which to execute statement
   * @param cmd - sql query to be executed
   * @param command_name - which command executes a query
   * @param error_generator functions which must generate error message
   * Functions are passed instead of string to avoid overhead of string
   * construction in successful case.
   * @return CommandResult with command name and error message
   */
  iroha::ametsuchi::CommandResult executeQuery(
      soci::session &sql,
      const std::string &cmd,
      const std::string &command_name,
      std::vector<std::function<std::string()>> &error_generator) noexcept {
    uint32_t result;
    try {
      sql << cmd, soci::into(result);
      if (result != 0) {
        return makeCommandError(error_generator[result - 1](), command_name);
      }
      return {};
    } catch (std::exception &e) {
      return makeCommandError(e.what(), command_name);
    }
  }

  std::string checkAccountRolePermission(
      shared_model::interface::permissions::Role permission,
      const shared_model::interface::types::AccountIdType &account_id) {
    const auto perm_str =
        shared_model::interface::RolePermissionSet({permission}).toBitstring();
    const auto bits = shared_model::interface::RolePermissionSet::size();
    std::string query = (boost::format(R"(
          SELECT COALESCE(bit_or(rp.permission), '0'::bit(%1%))
          & '%2%' = '%2%' FROM role_has_permissions AS rp
              JOIN account_has_roles AS ar on ar.role_id = rp.role_id
              WHERE ar.account_id = %3%)")
                         % bits % perm_str % account_id)
                            .str();
    return query;
  }

  std::string checkAccountGrantablePermission(
      shared_model::interface::permissions::Grantable permission,
      const shared_model::interface::types::AccountIdType &creator_id,
      const shared_model::interface::types::AccountIdType &account_id) {
    const auto perm_str =
        shared_model::interface::GrantablePermissionSet({permission})
            .toBitstring();
    const auto bits = shared_model::interface::GrantablePermissionSet::size();
    std::string query = (boost::format(R"(
          SELECT COALESCE(bit_or(permission), '0'::bit(%1%))
          & '%2%' = '%2%' FROM account_has_grantable_permissions
              WHERE account_id = %4% AND
              permittee_account_id = %3%
          )") % bits % perm_str
                         % creator_id % account_id)
                            .str();
    return query;
  }

  std::string checkAccountHasRoleOrGrantablePerm(
      shared_model::interface::permissions::Role role,
      shared_model::interface::permissions::Grantable grantable,
      const shared_model::interface::types::AccountIdType &creator_id,
      const shared_model::interface::types::AccountIdType &account_id) {
    return (boost::format(R"(WITH
          has_role_perm AS (%s),
          has_grantable_perm AS (%s)
          SELECT CASE
                           WHEN (SELECT * FROM has_grantable_perm) THEN true
                           WHEN (%s = %s) THEN
                               CASE
                                   WHEN (SELECT * FROM has_role_perm) THEN true
                                   ELSE false
                                END
                           ELSE false END
          )")
            % checkAccountRolePermission(role, creator_id)
            % checkAccountGrantablePermission(grantable, creator_id, account_id)
            % creator_id % account_id)
        .str();
  }

  std::string missRolePerm(
      shared_model::interface::types::AccountIdType account,
      shared_model::interface::permissions::Role perm) {
    return (boost::format("command validation failed: account %s"
                          " does not have permission %s (role)")
            % account % shared_model::proto::permissions::toString(perm))
        .str();
  }

  std::string missGrantablePerm(
      shared_model::interface::types::AccountIdType account,
      shared_model::interface::types::AccountIdType permittee,
      shared_model::interface::permissions::Grantable perm) {
    return (boost::format(
                "command validation failed: account %s"
                " does not have permission %s (grantable) for account %s")
            % account % shared_model::proto::permissions::toString(perm)
            % permittee)
        .str();
  }

  std::string missRoleOrGrantablePerm(
      shared_model::interface::types::AccountIdType account,
      shared_model::interface::types::AccountIdType permittee,
      shared_model::interface::permissions::Role role_perm,
      shared_model::interface::permissions::Grantable grantable_perm) {
    return (boost::format("command validation failed: account %s"
                          " does not have permission %s (role)"
                          " and permission %s (grantable) for account %s")
            % account % shared_model::proto::permissions::toString(role_perm)
            % shared_model::proto::permissions::toString(grantable_perm)
            % permittee)
        .str();
  }

  template <typename Format>
  void appendCommandName(const std::string &name,
                         Format &cmd,
                         bool do_validation) {
    auto command_name = name
        + (do_validation ? PreparedStatement::validationPrefix
                         : PreparedStatement::noValidationPrefix);
    cmd % command_name;
  }
}  // namespace

namespace iroha {
  namespace ametsuchi {
    const std::string PostgresCommandExecutor::addAssetQuantityBase = R"(
          PREPARE %s (text, text, int, text) AS
          WITH has_account AS (SELECT account_id FROM account
                               WHERE account_id = $1 LIMIT 1),
               has_asset AS (SELECT asset_id FROM asset
                             WHERE asset_id = $2 AND
                             precision >= $3 LIMIT 1),
               %s
               amount AS (SELECT amount FROM account_has_asset
                          WHERE asset_id = $2 AND
                          account_id = $1 LIMIT 1),
               new_value AS (SELECT $4::decimal +
                              (SELECT
                                  CASE WHEN EXISTS
                                      (SELECT amount FROM amount LIMIT 1) THEN
                                      (SELECT amount FROM amount LIMIT 1)
                                  ELSE 0::decimal
                              END) AS value
                          ),
               inserted AS
               (
                  INSERT INTO account_has_asset(account_id, asset_id, amount)
                  (
                      SELECT $1, $2, value FROM new_value
                      WHERE EXISTS (SELECT * FROM has_account LIMIT 1) AND
                        EXISTS (SELECT * FROM has_asset LIMIT 1) AND
                        EXISTS (SELECT value FROM new_value
                                WHERE value < 2::decimal ^ (256 - $3)
                                LIMIT 1)
                        %s
                  )
                  ON CONFLICT (account_id, asset_id) DO UPDATE
                  SET amount = EXCLUDED.amount
                  RETURNING (1)
               )
          SELECT CASE
              WHEN EXISTS (SELECT * FROM inserted LIMIT 1) THEN 0
              %s
              WHEN NOT EXISTS (SELECT * FROM has_account LIMIT 1) THEN 2
              WHEN NOT EXISTS (SELECT * FROM has_asset LIMIT 1) THEN 3
              WHEN NOT EXISTS (SELECT value FROM new_value
                               WHERE value < 2::decimal ^ (256 - $3)
                               LIMIT 1) THEN 4
              ELSE 5
          END AS result;)";

    const std::string PostgresCommandExecutor::addPeerBase = R"(
          PREPARE %s (text, text, text) AS
          WITH
          %s
          inserted AS (
              INSERT INTO peer(public_key, address)
              (
                  SELECT $2, $3
                  %s
              ) RETURNING (1)
          )
          SELECT CASE WHEN EXISTS (SELECT * FROM inserted) THEN 0
              %s
              ELSE 2 END AS result)";

    const std::string PostgresCommandExecutor::addSignatoryBase = R"(
          PREPARE %s (text, text, text) AS
          WITH %s
          insert_signatory AS
          (
              INSERT INTO signatory(public_key)
              (SELECT $3 %s) ON CONFLICT DO NOTHING RETURNING (1)
          ),
          has_signatory AS (SELECT * FROM signatory WHERE public_key = $3),
          insert_account_signatory AS
          (
              INSERT INTO account_has_signatory(account_id, public_key)
              (
                  SELECT $2, $3 WHERE (EXISTS
                  (SELECT * FROM insert_signatory) OR
                  EXISTS (SELECT * FROM has_signatory))
                  %s
              )
              RETURNING (1)
          )
          SELECT CASE
              WHEN EXISTS (SELECT * FROM insert_account_signatory) THEN 0
              %s
              WHEN EXISTS (SELECT * FROM insert_signatory) THEN 2
              ELSE 3
          END AS RESULT;)";

    const std::string PostgresCommandExecutor::appendRoleBase = R"(
            PREPARE %s (text, text, text) AS
            WITH %s
            inserted AS (
                INSERT INTO account_has_roles(account_id, role_id)
                (
                    SELECT $2, $3 %s) RETURNING (1)
            )
            SELECT CASE
                WHEN EXISTS (SELECT * FROM inserted) THEN 0
                %s
                ELSE 4
            END AS result)";

    const std::string PostgresCommandExecutor::createAccountBase = R"(
          PREPARE %s (text, text, text, text) AS
          WITH get_domain_default_role AS (SELECT default_role FROM domain
                                           WHERE domain_id = $3),
          %s
          insert_signatory AS
          (
              INSERT INTO signatory(public_key)
              (
                  SELECT $4 WHERE EXISTS
                  (SELECT * FROM get_domain_default_role)
              ) ON CONFLICT DO NOTHING RETURNING (1)
          ),
          has_signatory AS (SELECT * FROM signatory WHERE public_key = $4),
          insert_account AS
          (
              INSERT INTO account(account_id, domain_id, quorum, data)
              (
                  SELECT $2, $3, 1, '{}' WHERE (EXISTS
                      (SELECT * FROM insert_signatory) OR EXISTS
                      (SELECT * FROM has_signatory)
                  ) AND EXISTS (SELECT * FROM get_domain_default_role)
                  %s
              ) RETURNING (1)
          ),
          insert_account_signatory AS
          (
              INSERT INTO account_has_signatory(account_id, public_key)
              (
                  SELECT $2, $4 WHERE
                     EXISTS (SELECT * FROM insert_account)
              )
              RETURNING (1)
          ),
          insert_account_role AS
          (
              INSERT INTO account_has_roles(account_id, role_id)
              (
                  SELECT $2, default_role FROM get_domain_default_role
                  WHERE EXISTS (SELECT * FROM get_domain_default_role)
                    AND EXISTS (SELECT * FROM insert_account_signatory)
              ) RETURNING (1)
          )
          SELECT CASE
              WHEN EXISTS (SELECT * FROM insert_account_role) THEN 0
              %s
              WHEN NOT EXISTS (SELECT * FROM account
                               WHERE account_id = $2) THEN 2
              WHEN NOT EXISTS (SELECT * FROM account_has_signatory
                               WHERE account_id = $2
                               AND public_key = $4) THEN 3
              WHEN NOT EXISTS (SELECT * FROM account_has_roles
                               WHERE account_id = account_id AND role_id = (
                               SELECT default_role FROM get_domain_default_role)
                               ) THEN 4
              ELSE 5
              END AS result)";

    const std::string PostgresCommandExecutor::createAssetBase = R"(
              PREPARE %s (text, text, text, int) AS
              WITH %s
              inserted AS
              (
                  INSERT INTO asset(asset_id, domain_id, precision, data)
                  (
                      SELECT $2, $3, $4, NULL
                      %s
                  ) RETURNING (1)
              )
              SELECT CASE WHEN EXISTS (SELECT * FROM inserted) THEN 0
              %s
              ELSE 2 END AS result)";

    const std::string PostgresCommandExecutor::createDomainBase = R"(
              PREPARE %s (text, text, text) AS
              WITH %s
              inserted AS
              (
                  INSERT INTO domain(domain_id, default_role)
                  (
                      SELECT $2, $3
                      %s
                  ) RETURNING (1)
              )
              SELECT CASE WHEN EXISTS (SELECT * FROM inserted) THEN 0
              %s
              ELSE 2 END AS result)";

    const std::string PostgresCommandExecutor::createRoleBase = R"(
          PREPARE %s (text, text, bit) AS
          WITH %s
          insert_role AS (INSERT INTO role(role_id)
                              (SELECT $2
                              %s) RETURNING (1)),
          insert_role_permissions AS
          (
              INSERT INTO role_has_permissions(role_id, permission)
              (
                  SELECT $2, $3 WHERE EXISTS
                      (SELECT * FROM insert_role)
              ) RETURNING (1)
          )
          SELECT CASE
              WHEN EXISTS (SELECT * FROM insert_role_permissions) THEN 0
              %s
              WHEN EXISTS (SELECT * FROM role WHERE role_id = $2) THEN 1
              ELSE 4
              END AS result)";

    const std::string PostgresCommandExecutor::detachRoleBase = R"(
            PREPARE %s (text, text, text) AS
            WITH %s
            deleted AS
            (
              DELETE FROM account_has_roles
              WHERE account_id=$2
              AND role_id=$3
              %s
              RETURNING (1)
            )
            SELECT CASE WHEN EXISTS (SELECT * FROM deleted) THEN 0
            %s
            ELSE 2 END AS result)";

    const std::string PostgresCommandExecutor::grantPermissionBase = R"(
          PREPARE %s (text, text, bit, bit) AS
          WITH %s
            inserted AS (
              INSERT INTO account_has_grantable_permissions AS
              has_perm(permittee_account_id, account_id, permission)
              (SELECT $2, $1, $3 %s) ON CONFLICT
              (permittee_account_id, account_id)
              DO UPDATE SET permission=(SELECT has_perm.permission | $3
              WHERE (has_perm.permission & $3) <> $3)
              RETURNING (1)
            )
            SELECT CASE WHEN EXISTS (SELECT * FROM inserted) THEN 0
              %s
              ELSE 2 END AS result)";

    const std::string PostgresCommandExecutor::removeSignatoryBase = R"(
          PREPARE %s (text, text, text) AS
          WITH
          %s
          delete_account_signatory AS (DELETE FROM account_has_signatory
              WHERE account_id = $2
              AND public_key = $3
              %s
              RETURNING (1)),
          delete_signatory AS
          (
              DELETE FROM signatory WHERE public_key = $3 AND
                  NOT EXISTS (SELECT 1 FROM account_has_signatory
                              WHERE public_key = $3)
                  AND NOT EXISTS (SELECT 1 FROM peer WHERE public_key = $3)
              RETURNING (1)
          )
          SELECT CASE
              WHEN EXISTS (SELECT * FROM delete_account_signatory) THEN
              CASE
                  WHEN EXISTS (SELECT * FROM delete_signatory) THEN 0
                  WHEN EXISTS (SELECT 1 FROM account_has_signatory
                               WHERE public_key = $3) THEN 0
                  WHEN EXISTS (SELECT 1 FROM peer
                               WHERE public_key = $3) THEN 0
                  ELSE 2
              END
              %s
              ELSE 1
          END AS result)";

    const std::string PostgresCommandExecutor::revokePermissionBase = R"(
          PREPARE %s (text, text, bit, bit) AS
          WITH %s
              inserted AS (
                  UPDATE account_has_grantable_permissions as has_perm
                  SET permission=(SELECT has_perm.permission & $4
                  WHERE has_perm.permission & $3 = $3 AND
                  has_perm.permittee_account_id=$2 AND
                  has_perm.account_id=$1) WHERE
                  permittee_account_id=$2 AND
                  account_id=$1 %s
                RETURNING (1)
              )
              SELECT CASE WHEN EXISTS (SELECT * FROM inserted) THEN 0
                  %s
                  ELSE 2 END AS result)";

    const std::string PostgresCommandExecutor::setAccountDetailBase = R"(
          PREPARE %s (text, text, text[], text[], text, text) AS
          WITH %s
              inserted AS
              (
                  UPDATE account SET data = jsonb_set(
                  CASE WHEN data ?$1 THEN data ELSE
                  jsonb_set(data, $3, $6::jsonb) END,
                  $4, $5::jsonb) WHERE account_id=$2 %s
                  RETURNING (1)
              )
              SELECT CASE WHEN EXISTS (SELECT * FROM inserted) THEN 0
                  %s
                  ELSE 2 END AS result)";

    const std::string PostgresCommandExecutor::setQuorumBase = R"(
          PREPARE %s (text, text, int) AS
          WITH
          %s
          %s
          updated AS (
              UPDATE account SET quorum=$3
              WHERE account_id=$2
              %s
              RETURNING (1)
          )
          SELECT CASE WHEN EXISTS (SELECT * FROM updated) THEN 0
              %s
              ELSE 4
          END AS result)";

    const std::string PostgresCommandExecutor::subtractAssetQuantityBase = R"(
          PREPARE %s (text, text, int, text) AS
          WITH %s
               has_account AS (SELECT account_id FROM account
                               WHERE account_id = $1 LIMIT 1),
               has_asset AS (SELECT asset_id FROM asset
                             WHERE asset_id = $2
                             AND precision >= $3 LIMIT 1),
               amount AS (SELECT amount FROM account_has_asset
                          WHERE asset_id = $2
                          AND account_id = $1 LIMIT 1),
               new_value AS (SELECT
                              (SELECT
                                  CASE WHEN EXISTS
                                      (SELECT amount FROM amount LIMIT 1)
                                      THEN (SELECT amount FROM amount LIMIT 1)
                                  ELSE 0::decimal
                              END) - $4::decimal AS value
                          ),
               inserted AS
               (
                  INSERT INTO account_has_asset(account_id, asset_id, amount)
                  (
                      SELECT $1, $2, value FROM new_value
                      WHERE EXISTS (SELECT * FROM has_account LIMIT 1) AND
                        EXISTS (SELECT * FROM has_asset LIMIT 1) AND
                        EXISTS (SELECT value FROM new_value WHERE value >= 0 LIMIT 1)
                        %s
                  )
                  ON CONFLICT (account_id, asset_id)
                  DO UPDATE SET amount = EXCLUDED.amount
                  RETURNING (1)
               )
          SELECT CASE
              WHEN EXISTS (SELECT * FROM inserted LIMIT 1) THEN 0
              %s
              WHEN NOT EXISTS (SELECT * FROM has_account LIMIT 1) THEN 2
              WHEN NOT EXISTS (SELECT * FROM has_asset LIMIT 1) THEN 3
              WHEN NOT EXISTS
                  (SELECT value FROM new_value WHERE value >= 0 LIMIT 1) THEN 4
              ELSE 5
          END AS result)";

    const std::string PostgresCommandExecutor::transferAssetBase = R"(
          PREPARE %s (text, text, text, text, int, text) AS
          WITH
              %s
              has_src_account AS (SELECT account_id FROM account
                                   WHERE account_id = $2 LIMIT 1),
              has_dest_account AS (SELECT account_id FROM account
                                    WHERE account_id = $3
                                    LIMIT 1),
              has_asset AS (SELECT asset_id FROM asset
                             WHERE asset_id = $4 AND
                             precision >= $5 LIMIT 1),
              src_amount AS (SELECT amount FROM account_has_asset
                              WHERE asset_id = $4 AND
                              account_id = $2 LIMIT 1),
              dest_amount AS (SELECT amount FROM account_has_asset
                               WHERE asset_id = $4 AND
                               account_id = $3 LIMIT 1),
              new_src_value AS (SELECT
                              (SELECT
                                  CASE WHEN EXISTS
                                      (SELECT amount FROM src_amount LIMIT 1)
                                      THEN
                                      (SELECT amount FROM src_amount LIMIT 1)
                                  ELSE 0::decimal
                              END) - $6::decimal AS value
                          ),
              new_dest_value AS (SELECT
                              (SELECT $6::decimal +
                                  CASE WHEN EXISTS
                                      (SELECT amount FROM dest_amount LIMIT 1)
                                          THEN
                                      (SELECT amount FROM dest_amount LIMIT 1)
                                  ELSE 0::decimal
                              END) AS value
                          ),
              insert_src AS
              (
                  INSERT INTO account_has_asset(account_id, asset_id, amount)
                  (
                      SELECT $2, $4, value
                      FROM new_src_value
                      WHERE EXISTS (SELECT * FROM has_src_account LIMIT 1) AND
                        EXISTS (SELECT * FROM has_dest_account LIMIT 1) AND
                        EXISTS (SELECT * FROM has_asset LIMIT 1) AND
                        EXISTS (SELECT value FROM new_src_value
                                WHERE value >= 0 LIMIT 1) %s
                  )
                  ON CONFLICT (account_id, asset_id)
                  DO UPDATE SET amount = EXCLUDED.amount
                  RETURNING (1)
              ),
              insert_dest AS
              (
                  INSERT INTO account_has_asset(account_id, asset_id, amount)
                  (
                      SELECT $3, $4, value
                      FROM new_dest_value
                      WHERE EXISTS (SELECT * FROM insert_src) AND
                        EXISTS (SELECT * FROM has_src_account LIMIT 1) AND
                        EXISTS (SELECT * FROM has_dest_account LIMIT 1) AND
                        EXISTS (SELECT * FROM has_asset LIMIT 1) AND
                        EXISTS (SELECT value FROM new_dest_value
                                WHERE value < 2::decimal ^ (256 - $5)
                                LIMIT 1) %s
                  )
                  ON CONFLICT (account_id, asset_id)
                  DO UPDATE SET amount = EXCLUDED.amount
                  RETURNING (1)
               )
          SELECT CASE
              WHEN EXISTS (SELECT * FROM insert_dest LIMIT 1) THEN 0
              %s
              WHEN NOT EXISTS (SELECT * FROM has_dest_account LIMIT 1) THEN 2
              WHEN NOT EXISTS (SELECT * FROM has_src_account LIMIT 1) THEN 3
              WHEN NOT EXISTS (SELECT * FROM has_asset LIMIT 1) THEN 4
              WHEN NOT EXISTS (SELECT value FROM new_src_value
                               WHERE value >= 0 LIMIT 1) THEN 5
              WHEN NOT EXISTS (SELECT value FROM new_dest_value
                               WHERE value < 2::decimal ^ (256 - $5)
                               LIMIT 1) THEN 6
              ELSE 7
          END AS result)";

    std::string CommandError::toString() const {
      return (boost::format("%s: %s") % command_name % error_message).str();
    }

    PostgresCommandExecutor::PostgresCommandExecutor(soci::session &sql)
        : sql_(sql), do_validation_(true) {}

    void PostgresCommandExecutor::setCreatorAccountId(
        const shared_model::interface::types::AccountIdType
            &creator_account_id) {
      creator_account_id_ = creator_account_id;
    }

    void PostgresCommandExecutor::doValidation(bool do_validation) {
      do_validation_ = do_validation;
    }

    CommandResult PostgresCommandExecutor::operator()(
        const shared_model::interface::AddAssetQuantity &command) {
      auto &account_id = creator_account_id_;
      auto &asset_id = command.assetId();
      auto amount = command.amount().toStringRepr();
      int precision = command.amount().precision();

      // 14.09.2018 nickaleks: IR-1707 move common logic to separate function
      auto cmd = boost::format("EXECUTE %1% ('%2%', '%3%', %4%, '%5%')");

      appendCommandName("addAssetQuantity", cmd, do_validation_);

      cmd = (cmd % account_id % asset_id % precision % amount);

      std::vector<std::function<std::string()>> message_gen = {
          [&] {
            return missRolePerm(
                creator_account_id_,
                shared_model::interface::permissions::Role::kAddAssetQty);
          },
          [] { return std::string("Account does not exist"); },
          [] {
            return std::string("Asset with given precision does not exist");
          },
          [] { return std::string("Summation overflows uint256"); },
      };
      return executeQuery(sql_, cmd.str(), "AddAssetQuantity", message_gen);
    }

    CommandResult PostgresCommandExecutor::operator()(
        const shared_model::interface::AddPeer &command) {
      auto &peer = command.peer();

      auto cmd = boost::format("EXECUTE %1% ('%2%', '%3%', '%4%')");

      appendCommandName("addPeer", cmd, do_validation_);

      cmd = (cmd % creator_account_id_ % peer.pubkey().hex() % peer.address());

      std::vector<std::function<std::string()>> message_gen = {
          [&] {
            return missRolePerm(
                creator_account_id_,
                shared_model::interface::permissions::Role::kAddPeer);
          },
          [&] {
            return (boost::format("failed to insert peer, public key: '%s', "
                                  "address: '%s'")
                    % peer.pubkey().hex() % peer.address())
                .str();
          },
      };
      return executeQuery(sql_, cmd.str(), "AddPeer", message_gen);
    }

    CommandResult PostgresCommandExecutor::operator()(
        const shared_model::interface::AddSignatory &command) {
      auto &account_id = command.accountId();
      auto pubkey = command.pubkey().hex();
      auto cmd = boost::format("EXECUTE %1% ('%2%', '%3%', '%4%')");

      appendCommandName("addSignatory", cmd, do_validation_);

      cmd = (cmd % creator_account_id_ % account_id % pubkey);

      std::vector<std::function<std::string()>> message_gen = {
          [&] {
            return missRoleOrGrantablePerm(
                creator_account_id_,
                account_id,
                shared_model::interface::permissions::Role::kAddSignatory,
                shared_model::interface::permissions::Grantable::
                    kAddMySignatory);
          },
          [&] {
            return (boost::format(
                        "failed to insert account signatory, account id: "
                        "'%s', signatory hex string: '%s")
                    % account_id % pubkey)
                .str();
          },
          [&] {
            return (boost::format("failed to insert signatory, "
                                  "signatory hex string: '%s'")
                    % pubkey)
                .str();
          },
      };
      return executeQuery(sql_, cmd.str(), "AddSignatory", message_gen);
    }

    CommandResult PostgresCommandExecutor::operator()(
        const shared_model::interface::AppendRole &command) {
      auto &account_id = command.accountId();
      auto &role_name = command.roleName();
      auto cmd = boost::format("EXECUTE %1% ('%2%', '%3%', '%4%')");

      appendCommandName("appendRole", cmd, do_validation_);

      cmd = (cmd % creator_account_id_ % account_id % role_name);
      std::vector<std::function<std::string()>> message_gen = {
          [&] {
            return (boost::format("is valid command validation failed: no "
                                  "roles in account %s")
                    % creator_account_id_)
                .str();
          },
          [&] {
            return (boost::format(
                        "is valid command validation failed: account %s"
                        " does not have some of the permissions in a role %s")
                    % creator_account_id_ % command.roleName())
                .str();
          },
          [&] {
            return missRolePerm(
                creator_account_id_,
                shared_model::interface::permissions::Role::kAppendRole);
          },
          [&] {
            return (boost::format(
                        "failed to insert account role, account: '%s', "
                        "role name: '%s'")
                    % account_id % role_name)
                .str();
          },
      };
      return executeQuery(sql_, cmd.str(), "AppendRole", message_gen);
    }

    CommandResult PostgresCommandExecutor::operator()(
        const shared_model::interface::CreateAccount &command) {
      auto &account_name = command.accountName();
      auto &domain_id = command.domainId();
      auto &pubkey = command.pubkey().hex();
      shared_model::interface::types::AccountIdType account_id =
          account_name + "@" + domain_id;

      auto cmd = boost::format("EXECUTE %1% ('%2%', '%3%', '%4%', '%5%')");

      appendCommandName("createAccount", cmd, do_validation_);

      cmd = (cmd % creator_account_id_ % account_id % domain_id % pubkey);

      std::vector<std::function<std::string()>> message_gen = {
          [&] {
            return missRolePerm(
                creator_account_id_,
                shared_model::interface::permissions::Role::kCreateAccount);
          },
          [&] {
            return (boost::format("failed to insert account, "
                                  "account id: '%s', "
                                  "domain id: '%s', "
                                  "quorum: '1', "
                                  "json_data: {}")
                    % account_id % domain_id)
                .str();
          },
          [&] {
            return (boost::format("failed to insert account signatory, "
                                  "account id: "
                                  "'%s', signatory hex string: '%s")
                    % account_id % pubkey)
                .str();
          },
          [&] {
            return (boost::format(
                        "failed to insert account role, account: '%s' "
                        "with default domain role name for domain: "
                        "'%s'")
                    % account_id % domain_id)
                .str();
          },
      };
      return executeQuery(sql_, cmd.str(), "CreateAccount", message_gen);
    }

    CommandResult PostgresCommandExecutor::operator()(
        const shared_model::interface::CreateAsset &command) {
      auto &domain_id = command.domainId();
      auto asset_id = command.assetName() + "#" + domain_id;
      int precision = command.precision();
      auto cmd = boost::format("EXECUTE %1% ('%2%', '%3%', '%4%', %5%)");

      appendCommandName("createAsset", cmd, do_validation_);

      cmd = (cmd % creator_account_id_ % asset_id % domain_id % precision);

      std::vector<std::function<std::string()>> message_gen = {
          [&] {
            return missRolePerm(
                creator_account_id_,
                shared_model::interface::permissions::Role::kCreateDomain);
          },
          [&] {
            return (boost::format("failed to insert asset, asset id: '%s', "
                                  "domain id: '%s', precision: %d")
                    % asset_id % domain_id % precision)
                .str();
          }};
      return executeQuery(sql_, cmd.str(), "CreateAsset", message_gen);
    }

    CommandResult PostgresCommandExecutor::operator()(
        const shared_model::interface::CreateDomain &command) {
      auto &domain_id = command.domainId();
      auto &default_role = command.userDefaultRole();
      auto cmd = boost::format("EXECUTE %1% ('%2%', '%3%', '%4%')");

      appendCommandName("createDomain", cmd, do_validation_);

      cmd = (cmd % creator_account_id_ % domain_id % default_role);
      std::vector<std::function<std::string()>> message_gen = {
          [&] {
            return missRolePerm(
                creator_account_id_,
                shared_model::interface::permissions::Role::kCreateDomain);
          },
          [&] {
            return (boost::format("failed to insert domain, domain id: '%s', "
                                  "default role: '%s'")
                    % domain_id % default_role)
                .str();
          }};
      return executeQuery(sql_, cmd.str(), "CreateDomain", message_gen);
    }

    CommandResult PostgresCommandExecutor::operator()(
        const shared_model::interface::CreateRole &command) {
      auto &role_id = command.roleName();
      auto &permissions = command.rolePermissions();
      auto perm_str = permissions.toBitstring();
      auto cmd = boost::format("EXECUTE %1% ('%2%', '%3%', '%4%')");

      appendCommandName("createRole", cmd, do_validation_);

      cmd = (cmd % creator_account_id_ % role_id % perm_str);
      std::vector<std::function<std::string()>> message_gen = {
          [&] {
            // TODO(@l4l) 26/06/18 need to be simplified at IR-1479
            const auto &str =
                shared_model::proto::permissions::toString(permissions);
            const auto perm_debug_str =
                std::accumulate(str.begin(), str.end(), std::string());
            return (boost::format("failed to insert role permissions, role "
                                  "id: '%s', permissions: [%s]")
                    % role_id % perm_debug_str)
                .str();
          },
          [&] {
            return (boost::format(
                        "is valid command validation failed: account %s"
                        " does not have some of the permissions from a role %s")
                    % creator_account_id_ % command.roleName())
                .str();
          },
          [&] {
            return missRolePerm(
                creator_account_id_,
                shared_model::interface::permissions::Role::kCreateRole);
          },
          [&] {
            return (boost::format("failed to insert role: '%s'") % role_id)
                .str();
          },
      };
      return executeQuery(sql_, cmd.str(), "CreateRole", message_gen);
    }

    CommandResult PostgresCommandExecutor::operator()(
        const shared_model::interface::DetachRole &command) {
      auto &account_id = command.accountId();
      auto &role_name = command.roleName();
      auto cmd = boost::format("EXECUTE %1% ('%2%', '%3%', '%4%')");

      appendCommandName("detachRole", cmd, do_validation_);

      cmd = (cmd % creator_account_id_ % account_id % role_name);
      std::vector<std::function<std::string()>> message_gen = {
          [&] {
            return missRolePerm(
                creator_account_id_,
                shared_model::interface::permissions::Role::kDetachRole);
          },
          [&] {
            return (boost::format(
                        "failed to delete account role, account id: '%s', "
                        "role name: '%s'")
                    % account_id % role_name)
                .str();
          }};
      return executeQuery(sql_, cmd.str(), "DetachRole", message_gen);
    }

    CommandResult PostgresCommandExecutor::operator()(
        const shared_model::interface::GrantPermission &command) {
      auto &permittee_account_id = command.accountId();
      auto permission = command.permissionName();
      auto perm = shared_model::interface::RolePermissionSet(
                      {shared_model::interface::permissions::permissionFor(
                          command.permissionName())})
                      .toBitstring();
      const auto perm_str =
          shared_model::interface::GrantablePermissionSet({permission})
              .toBitstring();
      auto cmd = boost::format("EXECUTE %1% ('%2%', '%3%', '%4%', '%5%')");

      appendCommandName("grantPermission", cmd, do_validation_);

      cmd =
          (cmd % creator_account_id_ % permittee_account_id % perm_str % perm);
      std::vector<std::function<std::string()>> message_gen = {
          [&] {
            return missGrantablePerm(creator_account_id_,
                                     permittee_account_id,
                                     command.permissionName());
          },
          [&] {
            return (boost::format(
                        "failed to insert account grantable permission, "
                        "permittee account id: '%s', "
                        "account id: '%s', "
                        "permission: '%s'")
                    % permittee_account_id
                    % creator_account_id_
                    // TODO(@l4l) 26/06/18 need to be simplified at IR-1479
                    % shared_model::proto::permissions::toString(permission))
                .str();
          }};

      return executeQuery(sql_, cmd.str(), "GrantPermission", message_gen);
    }

    CommandResult PostgresCommandExecutor::operator()(
        const shared_model::interface::RemoveSignatory &command) {
      auto &account_id = command.accountId();
      auto &pubkey = command.pubkey().hex();
      auto cmd = boost::format("EXECUTE %1% ('%2%', '%3%', '%4%')");

      appendCommandName("removeSignatory", cmd, do_validation_);

      cmd = (cmd % creator_account_id_ % account_id % pubkey);
      std::vector<std::function<std::string()>> message_gen = {
          [&] {
            return (boost::format(
                        "failed to delete account signatory, account id: "
                        "'%s', signatory hex string: '%s'")
                    % account_id % pubkey)
                .str();
          },
          [&] {
            return (boost::format("failed to delete signatory, "
                                  "signatory hex string: '%s'")
                    % pubkey)
                .str();
          },
          [&] {
            return (boost::format(
                        "command validation failed: no account %s found")
                    % command.accountId())
                .str();
          },
          [&] {
            return (boost::format(
                        "command validation failed: no signatories in "
                        "account %s found")
                    % command.accountId())
                .str();
          },
          [&] {
            return "command validation failed: size of rest "
                   "signatories "
                   "becomes less than the quorum";
          },
          [&] {
            return missRoleOrGrantablePerm(
                creator_account_id_,
                command.accountId(),
                shared_model::interface::permissions::Role::kRemoveSignatory,
                shared_model::interface::permissions::Grantable::
                    kRemoveMySignatory);
          },
      };
      return executeQuery(sql_, cmd.str(), "RemoveSignatory", message_gen);
    }

    CommandResult PostgresCommandExecutor::operator()(
        const shared_model::interface::RevokePermission &command) {
      auto &permittee_account_id = command.accountId();
      auto &account_id = creator_account_id_;
      auto permission = command.permissionName();
      const auto without_perm_str =
          shared_model::interface::GrantablePermissionSet()
              .set()
              .unset(permission)
              .toBitstring();
      const auto perms = shared_model::interface::GrantablePermissionSet()
                             .set(permission)
                             .toBitstring();

      auto cmd = boost::format("EXECUTE %1% ('%2%', '%3%', '%4%', '%5%')");

      appendCommandName("revokePermission", cmd, do_validation_);

      cmd = (cmd % creator_account_id_ % permittee_account_id % perms
             % without_perm_str);

      std::vector<std::function<std::string()>> message_gen = {
          [&] {
            return missGrantablePerm(creator_account_id_,
                                     command.accountId(),
                                     command.permissionName());
          },
          [&] {
            return (boost::format(
                        "failed to delete account grantable permission, "
                        "permittee account id: '%s', "
                        "account id: '%s', "
                        "permission id: '%s'")
                    % permittee_account_id
                    % account_id
                    // TODO(@l4l) 26/06/18 need to be simplified at IR-1479
                    % shared_model::proto::permissions::toString(permission))
                .str();
          }};
      return executeQuery(sql_, cmd.str(), "RevokePermission", message_gen);
    }

    CommandResult PostgresCommandExecutor::operator()(
        const shared_model::interface::SetAccountDetail &command) {
      auto &account_id = command.accountId();
      auto &key = command.key();
      auto &value = command.value();
      if (creator_account_id_.empty()) {
        // When creator is not known, it is genesis block
        creator_account_id_ = "genesis";
      }
      std::string json = "{" + creator_account_id_ + "}";
      std::string empty_json = "{}";
      std::string filled_json = "{" + creator_account_id_ + ", " + key + "}";
      std::string val = "\"" + value + "\"";

      auto cmd = boost::format(
          "EXECUTE %1% ('%2%', '%3%', '%4%', '%5%', '%6%', '%7%')");

      appendCommandName("setAccountDetail", cmd, do_validation_);

      cmd = (cmd % creator_account_id_ % account_id % json % filled_json % val
             % empty_json);
      std::vector<std::function<std::string()>> message_gen = {
          [&] {
            return missRoleOrGrantablePerm(
                creator_account_id_,
                command.accountId(),
                shared_model::interface::permissions::Role::kSetDetail,
                shared_model::interface::permissions::Grantable::
                    kSetMyAccountDetail);
          },
          [&] {
            return (boost::format(
                        "failed to set account key-value, account id: '%s', "
                        "creator account id: '%s',\n key: '%s', value: '%s'")
                    % account_id % creator_account_id_ % key % value)
                .str();
          }};
      return executeQuery(sql_, cmd.str(), "SetAccountDetail", message_gen);
    }

    CommandResult PostgresCommandExecutor::operator()(
        const shared_model::interface::SetQuorum &command) {
      auto &account_id = command.accountId();
      int quorum = command.newQuorum();
      auto cmd = boost::format("EXECUTE %1% ('%2%', '%3%', %4%)");

      appendCommandName("setQuorum", cmd, do_validation_);

      cmd = (cmd % creator_account_id_ % account_id % quorum);
      std::vector<std::function<std::string()>> message_gen = {
          [&] {
            return (boost::format("is valid command validation failed: no "
                                  "signatories of an "
                                  "account %s found")
                    % account_id)
                .str();
          },
          [&] {
            return (boost::format(
                        "is valid command validation failed: account's %s"
                        " new quorum size is "
                        "out of bounds; "
                        "value is %s")
                    % account_id % std::to_string(quorum))
                .str();
          },
          [&] {
            return missRoleOrGrantablePerm(
                creator_account_id_,
                account_id,
                shared_model::interface::permissions::Role::kSetQuorum,
                shared_model::interface::permissions::Grantable::kSetMyQuorum);
          },
          [&] {
            return (boost::format("failed to update account, account id: '%s', "
                                  "quorum: '%s'")
                    % account_id % quorum)
                .str();
          },
      };
      return executeQuery(sql_, cmd.str(), "SetQuorum", message_gen);
    }

    CommandResult PostgresCommandExecutor::operator()(
        const shared_model::interface::SubtractAssetQuantity &command) {
      auto &asset_id = command.assetId();
      auto amount = command.amount().toStringRepr();
      uint32_t precision = command.amount().precision();
      auto cmd = boost::format("EXECUTE %1% ('%2%', '%3%', %4%, '%5%')");

      appendCommandName("subtractAssetQuantity", cmd, do_validation_);

      cmd = (cmd % creator_account_id_ % asset_id % precision % amount);

      std::vector<std::function<std::string()>> message_gen = {
          [&] {
            return missRolePerm(
                creator_account_id_,
                shared_model::interface::permissions::Role::kSubtractAssetQty);
          },
          [&] { return "Account does not exist with given precision"; },
          [&] { return "Asset with given precision does not exist"; },
          [&] { return "Subtracts overdrafts account asset"; },
      };
      return executeQuery(
          sql_, cmd.str(), "SubtractAssetQuantity", message_gen);
    }

    CommandResult PostgresCommandExecutor::operator()(
        const shared_model::interface::TransferAsset &command) {
      auto &src_account_id = command.srcAccountId();
      auto &dest_account_id = command.destAccountId();
      auto &asset_id = command.assetId();
      auto amount = command.amount().toStringRepr();
      uint32_t precision = command.amount().precision();
      auto cmd =
          boost::format("EXECUTE %1% ('%2%', '%3%', '%4%', '%5%', %6%, '%7%')");

      appendCommandName("transferAsset", cmd, do_validation_);

      cmd = (cmd % creator_account_id_ % src_account_id % dest_account_id
             % asset_id % precision % amount);
      std::vector<std::function<std::string()>> message_gen = {
          [&] {
            return (boost::format(
                        "has permission command validation failed: account %s"
                        " does not have permission %s"
                        " for account or does not have %s"
                        " for his own account or destination account %s"
                        " does not have %s")
                    % creator_account_id_
                    % shared_model::proto::permissions::toString(
                          shared_model::interface::permissions::Grantable::
                              kTransferMyAssets)
                    % shared_model::proto::permissions::toString(
                          shared_model::interface::permissions::Role::kTransfer)
                    % command.destAccountId()
                    % shared_model::proto::permissions::toString(
                          shared_model::interface::permissions::Role::kReceive))
                .str();
          },
          [&] { return "Destination account does not exist"; },
          [&] { return "Source account does not exist"; },
          [&] { return "Asset with given precision does not exist"; },
          [&] { return "Transfer overdrafts source account asset"; },
          [&] { return "Transfer overflows destanation account asset"; },
      };
      return executeQuery(sql_, cmd.str(), "TransferAsset", message_gen);
    }

    void PostgresCommandExecutor::prepareStatements(soci::session &sql) {
      std::vector<PreparedStatement> statements;

      statements.push_back(
          {"addAssetQuantity",
           addAssetQuantityBase,
           {(boost::format(R"(has_perm AS (%s),)")
             % checkAccountRolePermission(
                   shared_model::interface::permissions::Role::kAddAssetQty,
                   "$1"))
                .str(),
            "AND (SELECT * from has_perm)",
            "WHEN NOT (SELECT * from has_perm) THEN 1"}});

      statements.push_back(
          {"addPeer",
           addPeerBase,
           {(boost::format(R"(has_perm AS (%s),)")
             % checkAccountRolePermission(
                   shared_model::interface::permissions::Role::kAddPeer, "$1"))
                .str(),
            "WHERE (SELECT * FROM has_perm)",
            "WHEN NOT (SELECT * from has_perm) THEN 1"}});

      statements.push_back(
          {"addSignatory",
           addSignatoryBase,
           {(boost::format(R"(
                has_perm AS (%s),)")
             % checkAccountHasRoleOrGrantablePerm(
                   shared_model::interface::permissions::Role::kAddSignatory,
                   shared_model::interface::permissions::Grantable::
                       kAddMySignatory,
                   "$1",
                   "$2"))
                .str(),
            " WHERE (SELECT * FROM has_perm)",
            " AND (SELECT * FROM has_perm)",
            "WHEN NOT (SELECT * from has_perm) THEN 1"}});

      const auto bits = shared_model::interface::RolePermissionSet::size();
      const auto grantable_bits =
          shared_model::interface::GrantablePermissionSet::size();

      statements.push_back(
          {"appendRole",
           appendRoleBase,
           {(boost::format(R"(
            has_perm AS (%1%),
            role_permissions AS (
                SELECT permission FROM role_has_permissions
                WHERE role_id = $3
            ),
            account_roles AS (
                SELECT role_id FROM account_has_roles WHERE account_id = $1
            ),
            account_has_role_permissions AS (
                SELECT COALESCE(bit_or(rp.permission), '0'::bit(%2%)) &
                    (SELECT * FROM role_permissions) =
                    (SELECT * FROM role_permissions)
                FROM role_has_permissions AS rp
                JOIN account_has_roles AS ar on ar.role_id = rp.role_id
                WHERE ar.account_id = $1
            ),)")
             % checkAccountRolePermission(
                   shared_model::interface::permissions::Role::kAppendRole,
                   "$1")
             % bits)
                .str(),
            R"( WHERE
                    EXISTS (SELECT * FROM account_roles) AND
                    (SELECT * FROM account_has_role_permissions)
                    AND (SELECT * FROM has_perm))",
            R"(
                WHEN NOT EXISTS (SELECT * FROM account_roles) THEN 1
                WHEN NOT (SELECT * FROM account_has_role_permissions) THEN 2
                WHEN NOT (SELECT * FROM has_perm) THEN 3)"}});

      statements.push_back(
          {"createAccount",
           createAccountBase,
           {(boost::format(R"(
            has_perm AS (%s),)")
             % checkAccountRolePermission(
                   shared_model::interface::permissions::Role::kCreateAccount,
                   "$1"))
                .str(),
            R"(AND (SELECT * FROM has_perm))",
            R"(WHEN NOT (SELECT * FROM has_perm) THEN 1)"}});

      statements.push_back(
          {"createAsset",
           createAssetBase,
           {(boost::format(R"(
              has_perm AS (%s),)")
             % checkAccountRolePermission(
                   shared_model::interface::permissions::Role::kCreateAsset,
                   "$1"))
                .str(),
            R"(WHERE (SELECT * FROM has_perm))",
            R"(WHEN NOT (SELECT * FROM has_perm) THEN 1)"}});

      statements.push_back(
          {"createDomain",
           createDomainBase,
           {(boost::format(R"(
              has_perm AS (%s),)")
             % checkAccountRolePermission(
                   shared_model::interface::permissions::Role::kCreateDomain,
                   "$1"))
                .str(),
            R"(WHERE (SELECT * FROM has_perm))",
            R"(WHEN NOT (SELECT * FROM has_perm) THEN 1)"}});

      statements.push_back(
          {"createRole",
           createRoleBase,
           {(boost::format(R"(
          account_has_role_permissions AS (
                SELECT COALESCE(bit_or(rp.permission), '0'::bit(%s)) &
                    $3 = $3
                FROM role_has_permissions AS rp
                JOIN account_has_roles AS ar on ar.role_id = rp.role_id
                WHERE ar.account_id = $1),
          has_perm AS (%s),)")
             % bits
             % checkAccountRolePermission(
                   shared_model::interface::permissions::Role::kCreateRole,
                   "$1"))
                .str(),
            R"(WHERE (SELECT * FROM account_has_role_permissions)
                          AND (SELECT * FROM has_perm))",
            R"(WHEN NOT (SELECT * FROM
                               account_has_role_permissions) THEN 2
                        WHEN NOT (SELECT * FROM has_perm) THEN 3)"}});

      statements.push_back(
          {"detachRole",
           detachRoleBase,
           {(boost::format(R"(
            has_perm AS (%s),)")
             % checkAccountRolePermission(
                   shared_model::interface::permissions::Role::kDetachRole,
                   "$1"))
                .str(),
            R"(AND (SELECT * FROM has_perm))",
            R"(WHEN NOT (SELECT * FROM has_perm) THEN 1)"}});

      statements.push_back({"grantPermission",
                            grantPermissionBase,
                            {(boost::format(R"(
            has_perm AS (SELECT COALESCE(bit_or(rp.permission), '0'::bit(%1%))
          & $4 = $4 FROM role_has_permissions AS rp
              JOIN account_has_roles AS ar on ar.role_id = rp.role_id
              WHERE ar.account_id = $1),)")
                              % bits)
                                 .str(),
                             R"( WHERE (SELECT * FROM has_perm))",
                             R"(WHEN NOT (SELECT * FROM has_perm) THEN 1)"}});

      statements.push_back(
          {"removeSignatory",
           removeSignatoryBase,
           {(boost::format(R"(
          has_perm AS (%s),
          get_account AS (
              SELECT quorum FROM account WHERE account_id = $2 LIMIT 1
           ),
          get_signatories AS (
              SELECT public_key FROM account_has_signatory
              WHERE account_id = $2
          ),
          check_account_signatories AS (
              SELECT quorum FROM get_account
              WHERE quorum < (SELECT COUNT(*) FROM get_signatories)
          ),
          )")
             % checkAccountHasRoleOrGrantablePerm(
                   shared_model::interface::permissions::Role::kRemoveSignatory,
                   shared_model::interface::permissions::Grantable::
                       kRemoveMySignatory,
                   "$1",
                   "$2"))
                .str(),
            R"(
              AND (SELECT * FROM has_perm)
              AND EXISTS (SELECT * FROM get_account)
              AND EXISTS (SELECT * FROM get_signatories)
              AND EXISTS (SELECT * FROM check_account_signatories)
          )",
            R"(
              WHEN NOT EXISTS (SELECT * FROM get_account) THEN 3
              WHEN NOT (SELECT * FROM has_perm) THEN 6
              WHEN NOT EXISTS (SELECT * FROM get_signatories) THEN 4
              WHEN NOT EXISTS (SELECT * FROM check_account_signatories) THEN 5
          )"}});

      statements.push_back({"revokePermission",
                            revokePermissionBase,
                            {(boost::format(R"(
            has_perm AS (SELECT COALESCE(bit_or(permission), '0'::bit(%1%))
          & $3 = $3 FROM account_has_grantable_permissions
              WHERE account_id = $1 AND
              permittee_account_id = $2),)")
                              % grantable_bits)
                                 .str(),
                             R"( AND (SELECT * FROM has_perm))",
                             R"( WHEN NOT (SELECT * FROM has_perm) THEN 1 )"}});

      statements.push_back(
          {"setAccountDetail",
           setAccountDetailBase,
           {(boost::format(R"(
              has_role_perm AS (%s),
              has_grantable_perm AS (%s),
              has_perm AS (SELECT CASE
                               WHEN (SELECT * FROM has_grantable_perm) THEN true
                               WHEN ($1 = $2) THEN true
                               WHEN (SELECT * FROM has_role_perm) THEN true
                               ELSE false END
              ),
              )")
             % checkAccountRolePermission(
                   shared_model::interface::permissions::Role::kSetDetail, "$1")
             % checkAccountGrantablePermission(
                   shared_model::interface::permissions::Grantable::
                       kSetMyAccountDetail,
                   "$1",
                   "$2"))
                .str(),
            R"( AND (SELECT * FROM has_perm))",
            R"( WHEN NOT (SELECT * FROM has_perm) THEN 1 )"}});

      statements.push_back(
          {"setQuorum",
           setQuorumBase,
           {R"( get_signatories AS (
                    SELECT public_key FROM account_has_signatory
                    WHERE account_id = $2
                ),
                check_account_signatories AS (
                    SELECT 1 FROM account
                    WHERE $3 >= (SELECT COUNT(*) FROM get_signatories)
                    AND account_id = $2
                ),)",
            (boost::format(R"(
          has_perm AS (%s),)")
             % checkAccountHasRoleOrGrantablePerm(
                   shared_model::interface::permissions::Role::kSetQuorum,
                   shared_model::interface::permissions::Grantable::
                       kSetMyQuorum,
                   "$1",
                   "$2"))
                .str(),
            R"(AND EXISTS
              (SELECT * FROM get_signatories)
              AND EXISTS (SELECT * FROM check_account_signatories)
              AND (SELECT * FROM has_perm))",
            R"(
              WHEN NOT (SELECT * FROM has_perm) THEN 3
              WHEN NOT EXISTS (SELECT * FROM get_signatories) THEN 1
              WHEN NOT EXISTS (SELECT * FROM check_account_signatories) THEN 2
              )"}});

      statements.push_back(
          {"subtractAssetQuantity",
           subtractAssetQuantityBase,
           {(boost::format(R"(
               has_perm AS (%s),)")
             % checkAccountRolePermission(shared_model::interface::permissions::
                                              Role::kSubtractAssetQty,
                                          "$1"))
                .str(),
            R"( AND (SELECT * FROM has_perm))",
            R"( WHEN NOT (SELECT * FROM has_perm) THEN 1 )"}});

      statements.push_back(
          {"transferAsset",
           transferAssetBase,
           {(boost::format(R"(
              has_role_perm AS (%s),
              has_grantable_perm AS (%s),
              dest_can_receive AS (%s),
              has_perm AS (SELECT
                               CASE WHEN (SELECT * FROM dest_can_receive) THEN
                                   CASE WHEN NOT ($1 = $2) THEN
                                       CASE WHEN (SELECT * FROM has_grantable_perm)
                                           THEN true
                                       ELSE false END
                                   ELSE
                                        CASE WHEN (SELECT * FROM has_role_perm)
                                            THEN true
                                        ELSE false END
                                   END
                               ELSE false END
              ),
              )")
             % checkAccountRolePermission(
                   shared_model::interface::permissions::Role::kTransfer, "$1")
             % checkAccountGrantablePermission(
                   shared_model::interface::permissions::Grantable::
                       kTransferMyAssets,
                   "$1",
                   "$2")
             % checkAccountRolePermission(
                   shared_model::interface::permissions::Role::kReceive, "$3"))
                .str(),
            R"( AND (SELECT * FROM has_perm))",
            R"( AND (SELECT * FROM has_perm))",
            R"( WHEN NOT (SELECT * FROM has_perm) THEN 1 )"}});

      for (const auto &st : statements) {
        prepareStatement(sql, st);
      }
    };
  }  // namespace ametsuchi
}  // namespace iroha
