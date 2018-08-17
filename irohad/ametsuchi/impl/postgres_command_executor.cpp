/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ametsuchi/impl/postgres_command_executor.hpp"

#include <boost/format.hpp>

#include "ametsuchi/impl/soci_utils.hpp"
#include "backend/protobuf/permissions.hpp"
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
  iroha::expected::Error<iroha::ametsuchi::CommandError> makeCommandError(
      const std::string &error_message,
      const std::string &command_name) noexcept {
    return iroha::expected::makeError(
        iroha::ametsuchi::CommandError{command_name, error_message});
  }

  /**
   * Transforms soci statement to CommandResult,
   * which will have error message generated exception
   * Assums that statement query returns 0 in case of success or error code
   * @param result which can be received by calling execute_
   * @param error_generator functions which must generate error message
   * to be used as a return error.
   * Functions are passed instead of string to avoid overhead of string
   * construction in successful case.
   * @return CommandResult with combined error message
   * in case of result contains error
   */
  iroha::ametsuchi::CommandResult makeCommandResultByReturnedValue(
      soci::statement &st,
      const std::string &command_name,
      std::vector<std::function<std::string()>> &error_generator) noexcept {
    uint32_t result;
    st.exchange(soci::into(result));
    st.define_and_bind();
    try {
      st.execute(true);
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
      const std::string &account_alias = "role_account_id") {
    const auto perm_str =
        shared_model::interface::RolePermissionSet({permission}).toBitstring();
    const auto bits = shared_model::interface::RolePermissionSet::size();
    std::string query = (boost::format(R"(
          SELECT COALESCE(bit_or(rp.permission), '0'::bit(%1%))
          & '%2%' = '%2%' FROM role_has_permissions AS rp
              JOIN account_has_roles AS ar on ar.role_id = rp.role_id
              WHERE ar.account_id = :%3%)")
                         % bits % perm_str % account_alias)
                            .str();
    return query;
  }

  std::string checkAccountGrantablePermission(
      shared_model::interface::permissions::Grantable permission) {
    const auto perm_str =
        shared_model::interface::GrantablePermissionSet({permission})
            .toBitstring();
    const auto bits = shared_model::interface::GrantablePermissionSet::size();
    std::string query = (boost::format(R"(
          SELECT COALESCE(bit_or(permission), '0'::bit(%1%))
          & '%2%' = '%2%' FROM account_has_grantable_permissions
              WHERE account_id = :grantable_account_id AND
              permittee_account_id = :grantable_permittee_account_id
          )") % bits % perm_str)
                            .str();
    return query;
  }

  std::string checkAccountHasRoleOrGrantablePerm(
      shared_model::interface::permissions::Role role,
      shared_model::interface::permissions::Grantable grantable) {
    return (boost::format(R"(WITH
          has_role_perm AS (%s),
          has_grantable_perm AS (%s)
          SELECT CASE
                           WHEN (SELECT * FROM has_grantable_perm) THEN true
                           WHEN (:creator_id = :account_id) THEN
                               CASE
                                   WHEN (SELECT * FROM has_role_perm) THEN true
                                   ELSE false
                                END
                           ELSE false END
          )")
            % checkAccountRolePermission(role)
            % checkAccountGrantablePermission(grantable))
        .str();
  }

  inline std::string missRolePerm(
      shared_model::interface::types::AccountIdType account,
      shared_model::interface::permissions::Role perm) {
    return (boost::format("command validation failed: account %s"
                          " does not have permission %s (role)")
            % account % shared_model::proto::permissions::toString(perm))
        .str();
  }

  inline std::string missGrantablePerm(
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

  inline std::string missRoleOrGrantablePerm(
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
}  // namespace

namespace iroha {
  namespace ametsuchi {

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
      auto precision = command.amount().precision();
      boost::format cmd(R"(
          WITH has_account AS (SELECT account_id FROM account
                               WHERE account_id = :account_id LIMIT 1),
               has_asset AS (SELECT asset_id FROM asset
                             WHERE asset_id = :asset_id AND
                             precision >= :precision LIMIT 1),
               %s
               amount AS (SELECT amount FROM account_has_asset
                          WHERE asset_id = :asset_id AND
                          account_id = :account_id LIMIT 1),
               new_value AS (SELECT :new_value::decimal +
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
                      SELECT :account_id, :asset_id, value FROM new_value
                      WHERE EXISTS (SELECT * FROM has_account LIMIT 1) AND
                        EXISTS (SELECT * FROM has_asset LIMIT 1) AND
                        EXISTS (SELECT value FROM new_value
                                WHERE value < 2::decimal ^ (256 - :precision)
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
                               WHERE value < 2::decimal ^ (256 - :precision)
                               LIMIT 1) THEN 4
              ELSE 5
          END AS result;)");
      if (do_validation_) {
        cmd =
            (cmd
             % (boost::format(R"(has_perm AS (%s),)")
                % checkAccountRolePermission(
                      shared_model::interface::permissions::Role::kAddAssetQty))
                   .str()
             % "AND (SELECT * from has_perm)"
             % "WHEN NOT (SELECT * from has_perm) THEN 1");
      } else {
        cmd = (cmd % "" % "" % "");
      }

      soci::statement st = sql_.prepare << cmd.str();
      // clang-format on

      st.exchange(soci::use(account_id, "account_id"));
      st.exchange(soci::use(creator_account_id_, "role_account_id"));
      st.exchange(soci::use(asset_id, "asset_id"));
      st.exchange(soci::use(amount, "new_value"));
      st.exchange(soci::use(precision, "precision"));

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
      return makeCommandResultByReturnedValue(
          st, "AddAssetQuantity", message_gen);
    }

    CommandResult PostgresCommandExecutor::operator()(
        const shared_model::interface::AddPeer &command) {
      auto &peer = command.peer();
      boost::format cmd(R"(
          WITH
          %s
          inserted AS (
              INSERT INTO peer(public_key, address)
              (
                  SELECT :pk, :address
                  %s
              ) RETURNING (1)
          )
          SELECT CASE WHEN EXISTS (SELECT * FROM inserted) THEN 0
              %s
              ELSE 2 END AS result)");
      if (do_validation_) {
        cmd = (cmd
               % (boost::format(R"(has_perm AS (%s),)")
                  % checkAccountRolePermission(
                        shared_model::interface::permissions::Role::kAddPeer))
                     .str()
               % "WHERE (SELECT * FROM has_perm)"
               % "WHEN NOT (SELECT * from has_perm) THEN 1");
      } else {
        cmd = (cmd % "" % "" % "");
      }
      soci::statement st = sql_.prepare << cmd.str();
      st.exchange(soci::use(peer.pubkey().hex(), "pk"));
      st.exchange(soci::use(peer.address(), "address"));
      st.exchange(soci::use(creator_account_id_, "role_account_id"));
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
      return makeCommandResultByReturnedValue(st, "AddPeer", message_gen);
    }

    CommandResult PostgresCommandExecutor::operator()(
        const shared_model::interface::AddSignatory &command) {
      auto &account_id = command.accountId();
      auto pubkey = command.pubkey().hex();
      boost::format cmd(R"(
          WITH %s
          insert_signatory AS
          (
              INSERT INTO signatory(public_key)
              %s ON CONFLICT DO NOTHING RETURNING (1)
          ),
          has_signatory AS (SELECT * FROM signatory WHERE public_key = :pk),
          insert_account_signatory AS
          (
              INSERT INTO account_has_signatory(account_id, public_key)
              (
                  SELECT :account_id, :pk WHERE (EXISTS
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
          END AS RESULT;)");

      if (do_validation_) {
        cmd =
            (cmd
             % (boost::format(R"(
          has_perm AS (%s),)")
                % checkAccountHasRoleOrGrantablePerm(
                      shared_model::interface::permissions::Role::kAddSignatory,
                      shared_model::interface::permissions::Grantable::
                          kAddMySignatory))
                   .str()
             % "(SELECT :pk WHERE (SELECT * FROM has_perm))"
             % " AND (SELECT * FROM has_perm)"
             % "WHEN NOT (SELECT * from has_perm) THEN 1");
      } else {
        cmd = (cmd % "" % "(SELECT :pk)" % "" % "");
      }
      soci::statement st = sql_.prepare << cmd.str();
      st.exchange(soci::use(pubkey, "pk"));
      st.exchange(soci::use(account_id, "account_id"));
      st.exchange(soci::use(creator_account_id_, "creator_id"));
      st.exchange(soci::use(creator_account_id_, "role_account_id"));
      st.exchange(soci::use(account_id, "grantable_account_id"));
      st.exchange(
          soci::use(creator_account_id_, "grantable_permittee_account_id"));

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
      return makeCommandResultByReturnedValue(st, "AddSignatory", message_gen);
    }

    CommandResult PostgresCommandExecutor::operator()(
        const shared_model::interface::AppendRole &command) {
      auto &account_id = command.accountId();
      auto &role_name = command.roleName();
      const auto bits = shared_model::interface::RolePermissionSet::size();
      boost::format cmd(R"(
            WITH %s
            inserted AS (
                INSERT INTO account_has_roles(account_id, role_id)
                (
                    SELECT :account_id, :role_id %s) RETURNING (1)
            )
            SELECT CASE
                WHEN EXISTS (SELECT * FROM inserted) THEN 0
                %s
                ELSE 4
            END AS result)");
      if (do_validation_) {
        cmd = (cmd
               % (boost::format(R"(
            has_perm AS (%1%),
            role_permissions AS (
                SELECT permission FROM role_has_permissions
                WHERE role_id = :role_id
            ),

            account_roles AS (
                SELECT role_id FROM account_has_roles WHERE account_id = :creator_id
            ),
            account_has_role_permissions AS (
                SELECT COALESCE(bit_or(rp.permission), '0'::bit(%2%)) &
                    (SELECT * FROM role_permissions) =
                    (SELECT * FROM role_permissions)
                FROM role_has_permissions AS rp
                JOIN account_has_roles AS ar on ar.role_id = rp.role_id
                WHERE ar.account_id = :creator_id
            ),)")
                  % checkAccountRolePermission(
                        shared_model::interface::permissions::Role::kAppendRole)
                  % std::to_string(bits))
                     .str()
               % R"( WHERE
                    EXISTS (SELECT * FROM account_roles) AND
                    (SELECT * FROM account_has_role_permissions)
                    AND (SELECT * FROM has_perm))"
               % R"(
                WHEN NOT EXISTS (SELECT * FROM account_roles) THEN 1
                WHEN NOT (SELECT * FROM account_has_role_permissions) THEN 2
                WHEN NOT (SELECT * FROM has_perm) THEN 3)");
      } else {
        cmd = (cmd % "" % "" % "");
      }
      soci::statement st = sql_.prepare << cmd.str();
      st.exchange(soci::use(account_id, "account_id"));
      st.exchange(soci::use(role_name, "role_id"));
      st.exchange(soci::use(creator_account_id_, "creator_id"));
      st.exchange(soci::use(creator_account_id_, "role_account_id"));
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
      return makeCommandResultByReturnedValue(st, "AppendRole", message_gen);
    }

    CommandResult PostgresCommandExecutor::operator()(
        const shared_model::interface::CreateAccount &command) {
      auto &account_name = command.accountName();
      auto &domain_id = command.domainId();
      auto &pubkey = command.pubkey().hex();
      std::string account_id = account_name + "@" + domain_id;
      boost::format cmd(R"(
          WITH get_domain_default_role AS (SELECT default_role FROM domain
                                           WHERE domain_id = :domain_id),
          %s
          insert_signatory AS
          (
              INSERT INTO signatory(public_key)
              (
                  SELECT :pk WHERE EXISTS
                  (SELECT * FROM get_domain_default_role)
              ) ON CONFLICT DO NOTHING RETURNING (1)
          ),
          has_signatory AS (SELECT * FROM signatory WHERE public_key = :pk),
          insert_account AS
          (
              INSERT INTO account(account_id, domain_id, quorum, data)
              (
                  SELECT :account_id, :domain_id, 1, '{}' WHERE (EXISTS
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
                  SELECT :account_id, :pk WHERE
                     EXISTS (SELECT * FROM insert_account)
              )
              RETURNING (1)
          ),
          insert_account_role AS
          (
              INSERT INTO account_has_roles(account_id, role_id)
              (
                  SELECT :account_id, default_role FROM get_domain_default_role
                  WHERE EXISTS (SELECT * FROM get_domain_default_role)
                    AND EXISTS (SELECT * FROM insert_account_signatory)
              ) RETURNING (1)
          )
          SELECT CASE
              WHEN EXISTS (SELECT * FROM insert_account_role) THEN 0
              %s
              WHEN NOT EXISTS (SELECT * FROM account
                               WHERE account_id = :account_id) THEN 2
              WHEN NOT EXISTS (SELECT * FROM account_has_signatory
                               WHERE account_id = :account_id
                               AND public_key = :pk) THEN 3
              WHEN NOT EXISTS (SELECT * FROM account_has_roles
                               WHERE account_id = account_id AND role_id = (
                               SELECT default_role FROM get_domain_default_role)
                               ) THEN 4
              ELSE 5
              END AS result)");
      if (do_validation_) {
        cmd = (cmd
               % (boost::format(R"(
            has_perm AS (%s),)")
                  % checkAccountRolePermission(
                        shared_model::interface::permissions::Role::
                            kCreateAccount))
                     .str()
               % R"(AND (SELECT * FROM has_perm))"
               % R"(WHEN NOT (SELECT * FROM has_perm) THEN 1)");
      } else {
        cmd = (cmd % "" % "" % "");
      }
      soci::statement st = sql_.prepare << cmd.str();
      st.exchange(soci::use(account_id, "account_id"));
      st.exchange(soci::use(creator_account_id_, "role_account_id"));
      st.exchange(soci::use(domain_id, "domain_id"));
      st.exchange(soci::use(pubkey, "pk"));
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
      return makeCommandResultByReturnedValue(st, "CreateAccount", message_gen);
    }

    CommandResult PostgresCommandExecutor::operator()(
        const shared_model::interface::CreateAsset &command) {
      auto &domain_id = command.domainId();
      auto asset_id = command.assetName() + "#" + domain_id;
      auto precision = command.precision();
      boost::format cmd(R"(
              WITH %s
              inserted AS
              (
                  INSERT INTO asset(asset_id, domain_id, precision, data)
                  (
                      SELECT :id, :domain_id, :precision, NULL
                      %s
                  ) RETURNING (1)
              )
              SELECT CASE WHEN EXISTS (SELECT * FROM inserted) THEN 0
              %s
              ELSE 2 END AS result)");
      if (do_validation_) {
        cmd =
            (cmd
             % (boost::format(R"(
              has_perm AS (%s),)")
                % checkAccountRolePermission(
                      shared_model::interface::permissions::Role::kCreateAsset))
                   .str()
             % R"(WHERE (SELECT * FROM has_perm))"
             % R"(WHEN NOT (SELECT * FROM has_perm) THEN 1)");
      } else {
        cmd = (cmd % "" % "" % "");
      }
      soci::statement st = sql_.prepare << cmd.str();
      st.exchange(soci::use(asset_id, "id"));
      st.exchange(soci::use(domain_id, "domain_id"));
      st.exchange(soci::use(precision, "precision"));
      st.exchange(soci::use(creator_account_id_, "role_account_id"));
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
      return makeCommandResultByReturnedValue(st, "CreateAsset", message_gen);
    }

    CommandResult PostgresCommandExecutor::operator()(
        const shared_model::interface::CreateDomain &command) {
      auto &domain_id = command.domainId();
      auto &default_role = command.userDefaultRole();
      boost::format cmd(R"(
              WITH %s
              inserted AS
              (
                  INSERT INTO domain(domain_id, default_role)
                  (
                      SELECT :id, :role
                      %s
                  ) RETURNING (1)
              )
              SELECT CASE WHEN EXISTS (SELECT * FROM inserted) THEN 0
              %s
              ELSE 2 END AS result)");
      if (do_validation_) {
        cmd = (cmd
               % (boost::format(R"(
              has_perm AS (%s),)")
                  % checkAccountRolePermission(
                        shared_model::interface::permissions::Role::
                            kCreateDomain))
                     .str()
               % R"(WHERE (SELECT * FROM has_perm))"
               % R"(WHEN NOT (SELECT * FROM has_perm) THEN 1)");
      } else {
        cmd = (cmd % "" % "" % "");
      }
      soci::statement st = sql_.prepare << cmd.str();
      st.exchange(soci::use(domain_id, "id"));
      st.exchange(soci::use(default_role, "role"));
      st.exchange(soci::use(creator_account_id_, "role_account_id"));
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
      return makeCommandResultByReturnedValue(st, "CreateDomain", message_gen);
    }

    CommandResult PostgresCommandExecutor::operator()(
        const shared_model::interface::CreateRole &command) {
      auto &role_id = command.roleName();
      auto &permissions = command.rolePermissions();
      auto perm_str = permissions.toBitstring();
      const auto bits = shared_model::interface::RolePermissionSet::size();
      boost::format cmd(R"(
          WITH %s
          insert_role AS (INSERT INTO role(role_id)
                              (SELECT :role_id
                              %s) RETURNING (1)),
          insert_role_permissions AS
          (
              INSERT INTO role_has_permissions(role_id, permission)
              (
                  SELECT :role_id, :perms WHERE EXISTS
                      (SELECT * FROM insert_role)
              ) RETURNING (1)
          )
          SELECT CASE
              WHEN EXISTS (SELECT * FROM insert_role_permissions) THEN 0
              %s
              WHEN EXISTS (SELECT * FROM role WHERE role_id = :role_id) THEN 1
              ELSE 4
              END AS result)");
      if (do_validation_) {
        cmd =
            (cmd
             % (boost::format(R"(
          account_has_role_permissions AS (
                SELECT COALESCE(bit_or(rp.permission), '0'::bit(%s)) &
                    :perms = :perms
                FROM role_has_permissions AS rp
                JOIN account_has_roles AS ar on ar.role_id = rp.role_id
                WHERE ar.account_id = :creator_id),
          has_perm AS (%s),)")
                % std::to_string(bits)
                % checkAccountRolePermission(
                      shared_model::interface::permissions::Role::kCreateRole))
                   .str()
             % R"(WHERE (SELECT * FROM account_has_role_permissions)
                          AND (SELECT * FROM has_perm))"
             % R"(WHEN NOT (SELECT * FROM
                               account_has_role_permissions) THEN 2
                        WHEN NOT (SELECT * FROM has_perm) THEN 3)");
      } else {
        cmd = (cmd % "" % "" % "");
      }
      soci::statement st = sql_.prepare << cmd.str();
      st.exchange(soci::use(role_id, "role_id"));
      st.exchange(soci::use(creator_account_id_, "creator_id"));
      st.exchange(soci::use(creator_account_id_, "role_account_id"));
      st.exchange(soci::use(perm_str, "perms"));

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
      return makeCommandResultByReturnedValue(st, "CreateRole", message_gen);
    }

    CommandResult PostgresCommandExecutor::operator()(
        const shared_model::interface::DetachRole &command) {
      auto &account_id = command.accountId();
      auto &role_name = command.roleName();
      boost::format cmd(R"(
            WITH %s
            deleted AS
            (
              DELETE FROM account_has_roles
              WHERE account_id=:account_id
              AND role_id=:role_id
              %s
              RETURNING (1)
            )
            SELECT CASE WHEN EXISTS (SELECT * FROM deleted) THEN 0
            %s
            ELSE 2 END AS result)");
      if (do_validation_) {
        cmd =
            (cmd
             % (boost::format(R"(
            has_perm AS (%s),)")
                % checkAccountRolePermission(
                      shared_model::interface::permissions::Role::kDetachRole))
                   .str()
             % R"(AND (SELECT * FROM has_perm))"
             % R"(WHEN NOT (SELECT * FROM has_perm) THEN 1)");
      } else {
        cmd = (cmd % "" % "" % "");
      }
      soci::statement st = sql_.prepare << cmd.str();
      st.exchange(soci::use(account_id, "account_id"));
      st.exchange(soci::use(creator_account_id_, "role_account_id"));
      st.exchange(soci::use(role_name, "role_id"));
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
      return makeCommandResultByReturnedValue(st, "DetachRole", message_gen);
    }

    CommandResult PostgresCommandExecutor::operator()(
        const shared_model::interface::GrantPermission &command) {
      auto &permittee_account_id = command.accountId();
      auto &account_id = creator_account_id_;
      auto permission = command.permissionName();
      const auto perm_str =
          shared_model::interface::GrantablePermissionSet({permission})
              .toBitstring();
      boost::format cmd(R"(
            WITH %s
            inserted AS (
              INSERT INTO account_has_grantable_permissions as
              has_perm(permittee_account_id, account_id, permission)
              (SELECT :permittee_account_id, :account_id, :perms %s) ON CONFLICT
              (permittee_account_id, account_id)
              DO UPDATE SET permission=(SELECT has_perm.permission | :perms
              WHERE (has_perm.permission & :perms) <> :perms)
              RETURNING (1)
            )
            SELECT CASE WHEN EXISTS (SELECT * FROM inserted) THEN 0
              %s
              ELSE 2 END AS result)");
      if (do_validation_) {
        cmd = (cmd
               % (boost::format(R"(
            has_perm AS (%s),)")
                  % checkAccountRolePermission(
                        shared_model::interface::permissions::permissionFor(
                            command.permissionName())))
                     .str()
               % R"( WHERE (SELECT * FROM has_perm))"
               % R"(WHEN NOT (SELECT * FROM has_perm) THEN 1)");
      } else {
        cmd = (cmd % "" % "" % "");
      }
      soci::statement st = sql_.prepare << cmd.str();
      st.exchange(soci::use(permittee_account_id, "permittee_account_id"));
      st.exchange(soci::use(creator_account_id_, "role_account_id"));
      st.exchange(soci::use(account_id, "account_id"));
      st.exchange(soci::use(perm_str, "perms"));
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
                    % account_id
                    // TODO(@l4l) 26/06/18 need to be simplified at IR-1479
                    % shared_model::proto::permissions::toString(permission))
                .str();
          }};

      return makeCommandResultByReturnedValue(
          st, "GrantPermission", message_gen);
    }

    CommandResult PostgresCommandExecutor::operator()(
        const shared_model::interface::RemoveSignatory &command) {
      auto &account_id = command.accountId();
      auto &pubkey = command.pubkey().hex();
      boost::format cmd(R"(
          WITH
          %s
          delete_account_signatory AS (DELETE FROM account_has_signatory
              WHERE account_id = :account_id
              AND public_key = :pk
              %s
              RETURNING (1)),
          delete_signatory AS
          (
              DELETE FROM signatory WHERE public_key = :pk AND
                  NOT EXISTS (SELECT 1 FROM account_has_signatory
                              WHERE public_key = :pk)
                  AND NOT EXISTS (SELECT 1 FROM peer WHERE public_key = :pk)
              RETURNING (1)
          )
          SELECT CASE
              WHEN EXISTS (SELECT * FROM delete_account_signatory) THEN
              CASE
                  WHEN EXISTS (SELECT * FROM delete_signatory) THEN 0
                  WHEN EXISTS (SELECT 1 FROM account_has_signatory
                               WHERE public_key = :pk) THEN 0
                  WHEN EXISTS (SELECT 1 FROM peer
                               WHERE public_key = :pk) THEN 0
                  ELSE 2
              END
              %s
              ELSE 1
          END AS result)");
      if (do_validation_) {
        cmd = (cmd
               % (boost::format(R"(
          has_perm AS (%s),
          get_account AS (
              SELECT quorum FROM account WHERE account_id = :account_id LIMIT 1
           ),
          get_signatories AS (
              SELECT public_key FROM account_has_signatory
              WHERE account_id = :account_id
          ),
          check_account_signatories AS (
              SELECT quorum FROM get_account
              WHERE quorum < (SELECT COUNT(*) FROM get_signatories)
          ),
          )")
                  % checkAccountHasRoleOrGrantablePerm(
                        shared_model::interface::permissions::Role::
                            kRemoveSignatory,
                        shared_model::interface::permissions::Grantable::
                            kRemoveMySignatory))
                     .str()
               % R"(
              AND (SELECT * FROM has_perm)
              AND EXISTS (SELECT * FROM get_account)
              AND EXISTS (SELECT * FROM get_signatories)
              AND EXISTS (SELECT * FROM check_account_signatories)
          )" % R"(
              WHEN NOT (SELECT * FROM has_perm) THEN 6
              WHEN NOT EXISTS (SELECT * FROM get_account) THEN 3
              WHEN NOT EXISTS (SELECT * FROM get_signatories) THEN 4
              WHEN NOT EXISTS (SELECT * FROM check_account_signatories) THEN 5
          )");
      } else {
        cmd = (cmd % "" % "" % "");
      }
      soci::statement st = sql_.prepare << cmd.str();
      st.exchange(soci::use(account_id, "account_id"));
      st.exchange(soci::use(pubkey, "pk"));
      st.exchange(soci::use(creator_account_id_, "creator_id"));
      st.exchange(soci::use(creator_account_id_, "role_account_id"));
      st.exchange(soci::use(account_id, "grantable_account_id"));
      st.exchange(
          soci::use(creator_account_id_, "grantable_permittee_account_id"));

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
      return makeCommandResultByReturnedValue(
          st, "RemoveSignatory", message_gen);
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
      boost::format cmd(R"(
              WITH %s
              inserted AS (
                  UPDATE account_has_grantable_permissions as has_perm
                  SET permission=(SELECT has_perm.permission & :without_perm
                  WHERE has_perm.permission & :perm = :perm AND
                  has_perm.permittee_account_id=:permittee_account_id AND
                  has_perm.account_id=:account_id) WHERE
                  permittee_account_id=:permittee_account_id AND
                  account_id=:account_id %s
                RETURNING (1)
              )
              SELECT CASE WHEN EXISTS (SELECT * FROM inserted) THEN 0
                  %s
                  ELSE 2 END AS result)");
      if (do_validation_) {
        cmd = (cmd
               % (boost::format(R"(
            has_perm AS (%s),)")
                  % checkAccountGrantablePermission(permission))
                     .str()
               % R"( AND (SELECT * FROM has_perm))"
               % R"( WHEN NOT (SELECT * FROM has_perm) THEN 1 )");
      } else {
        cmd = (cmd % "" % "" % "");
      }
      soci::statement st = sql_.prepare << cmd.str();
      st.exchange(
          soci::use(permittee_account_id, "grantable_permittee_account_id"));
      st.exchange(soci::use(account_id, "grantable_account_id"));
      st.exchange(soci::use(permittee_account_id, "permittee_account_id"));
      st.exchange(soci::use(account_id, "account_id"));
      st.exchange(soci::use(without_perm_str, "without_perm"));
      st.exchange(soci::use(perms, "perm"));
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
      return makeCommandResultByReturnedValue(
          st, "RevokePermission", message_gen);
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

      boost::format cmd(R"(
              WITH %s
              inserted AS
              (
                  UPDATE account SET data = jsonb_set(
                  CASE WHEN data ?:creator_account_id THEN data ELSE
                  jsonb_set(data, :json, :empty_json) END,
                  :filled_json, :val) WHERE account_id=:account_id %s
                  RETURNING (1)
              )
              SELECT CASE WHEN EXISTS (SELECT * FROM inserted) THEN 0
                  %s
                  ELSE 2 END AS result)");
      if (do_validation_) {
        cmd = (cmd
               % (boost::format(R"(
              has_role_perm AS (%s),
              has_grantable_perm AS (%s),
              has_perm AS (SELECT CASE
                               WHEN (SELECT * FROM has_grantable_perm) THEN true
                               WHEN (:creator_id = :account_id) THEN true
                               WHEN (SELECT * FROM has_role_perm) THEN true
                               ELSE false END
              ),
              )")
                  % checkAccountRolePermission(
                        shared_model::interface::permissions::Role::kSetDetail)
                  % checkAccountGrantablePermission(
                        shared_model::interface::permissions::Grantable::
                            kSetMyAccountDetail))
                     .str()
               % R"( AND (SELECT * FROM has_perm))"
               % R"( WHEN NOT (SELECT * FROM has_perm) THEN 1 )");
      } else {
        cmd = (cmd % "" % "" % "");
      }
      soci::statement st = sql_.prepare << cmd.str();
      st.exchange(soci::use(creator_account_id_, "creator_account_id"));
      st.exchange(soci::use(json, "json"));
      st.exchange(soci::use(empty_json, "empty_json"));
      st.exchange(soci::use(filled_json, "filled_json"));
      st.exchange(soci::use(val, "val"));
      st.exchange(soci::use(account_id, "account_id"));
      st.exchange(soci::use(creator_account_id_, "role_account_id"));
      st.exchange(soci::use(creator_account_id_, "creator_id"));
      st.exchange(soci::use(account_id, "grantable_account_id"));
      st.exchange(
          soci::use(creator_account_id_, "grantable_permittee_account_id"));
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
      return makeCommandResultByReturnedValue(
          st, "SetAccountDetail", message_gen);
    }

    CommandResult PostgresCommandExecutor::operator()(
        const shared_model::interface::SetQuorum &command) {
      auto &account_id = command.accountId();
      auto quorum = command.newQuorum();
      boost::format cmd(R"(WITH
          %s
          %s
          updated AS (
              UPDATE account SET quorum=:quorum
              WHERE account_id=:account_id
              %s
              RETURNING (1)
          )
          SELECT CASE WHEN EXISTS (SELECT * FROM updated) THEN 0
              %s
              ELSE 4
          END AS result)");
      if (do_validation_) {
        cmd = (cmd % R"(
          get_signatories AS (
              SELECT public_key FROM account_has_signatory
              WHERE account_id = :account_id
          ),
          check_account_signatories AS (
              SELECT 1 FROM account
              WHERE :quorum >= (SELECT COUNT(*) FROM get_signatories)
              AND account_id = :account_id
          ),)"
               % (boost::format(R"(
          has_perm AS (%s),)")
                  % checkAccountHasRoleOrGrantablePerm(
                        shared_model::interface::permissions::Role::kSetQuorum,
                        shared_model::interface::permissions::Grantable::
                            kSetMyQuorum))
                     .str()
               % R"(AND EXISTS
              (SELECT * FROM get_signatories)
              AND EXISTS (SELECT * FROM check_account_signatories)
              AND (SELECT * FROM has_perm))"
               % R"(
              WHEN NOT (SELECT * FROM has_perm) THEN 3
              WHEN NOT EXISTS (SELECT * FROM get_signatories) THEN 1
              WHEN NOT EXISTS (SELECT * FROM check_account_signatories) THEN 2
              )");
      } else {
        cmd = (cmd % "" % "" % "" % "");
      }
      soci::statement st = sql_.prepare << cmd.str();
      st.exchange(soci::use(quorum, "quorum"));
      st.exchange(soci::use(account_id, "account_id"));
      st.exchange(soci::use(creator_account_id_, "creator_id"));
      st.exchange(soci::use(creator_account_id_, "role_account_id"));
      st.exchange(soci::use(account_id, "grantable_account_id"));
      st.exchange(
          soci::use(creator_account_id_, "grantable_permittee_account_id"));
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
      return makeCommandResultByReturnedValue(st, "SetQuorum", message_gen);
    }

    CommandResult PostgresCommandExecutor::operator()(
        const shared_model::interface::SubtractAssetQuantity &command) {
      auto &account_id = creator_account_id_;
      auto &asset_id = command.assetId();
      auto amount = command.amount().toStringRepr();
      uint32_t precision = command.amount().precision();
      boost::format cmd(R"(
          WITH %s
               has_account AS (SELECT account_id FROM account
                               WHERE account_id = :account_id LIMIT 1),
               has_asset AS (SELECT asset_id FROM asset
                             WHERE asset_id = :asset_id
                             AND precision >= :precision LIMIT 1),
               amount AS (SELECT amount FROM account_has_asset
                          WHERE asset_id = :asset_id
                          AND account_id = :account_id LIMIT 1),
               new_value AS (SELECT
                              (SELECT
                                  CASE WHEN EXISTS
                                      (SELECT amount FROM amount LIMIT 1)
                                      THEN (SELECT amount FROM amount LIMIT 1)
                                  ELSE 0::decimal
                              END) - :value::decimal AS value
                          ),
               inserted AS
               (
                  INSERT INTO account_has_asset(account_id, asset_id, amount)
                  (
                      SELECT :account_id, :asset_id, value FROM new_value
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
          END AS result;)");
      if (do_validation_) {
        cmd = (cmd
               % (boost::format(R"(
               has_perm AS (%s),)")
                  % checkAccountRolePermission(
                        shared_model::interface::permissions::Role::
                            kSubtractAssetQty))
                     .str()
               % R"( AND (SELECT * FROM has_perm))"
               % R"( WHEN NOT (SELECT * FROM has_perm) THEN 1 )");
      } else {
        cmd = (cmd % "" % "" % "");
      }

      soci::statement st = sql_.prepare << cmd.str();
      st.exchange(soci::use(account_id, "account_id"));
      st.exchange(soci::use(asset_id, "asset_id"));
      st.exchange(soci::use(amount, "value"));
      st.exchange(soci::use(precision, "precision"));
      st.exchange(soci::use(creator_account_id_, "role_account_id"));

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
      return makeCommandResultByReturnedValue(
          st, "SubtractAssetQuantity", message_gen);
    }

    CommandResult PostgresCommandExecutor::operator()(
        const shared_model::interface::TransferAsset &command) {
      auto &src_account_id = command.srcAccountId();
      auto &dest_account_id = command.destAccountId();
      auto &asset_id = command.assetId();
      auto amount = command.amount().toStringRepr();
      uint32_t precision = command.amount().precision();
      boost::format cmd(R"(
          WITH
              %s
              has_src_account AS (SELECT account_id FROM account
                                   WHERE account_id = :src_account_id LIMIT 1),
              has_dest_account AS (SELECT account_id FROM account
                                    WHERE account_id = :dest_account_id
                                    LIMIT 1),
              has_asset AS (SELECT asset_id FROM asset
                             WHERE asset_id = :asset_id AND
                             precision >= :precision LIMIT 1),
              src_amount AS (SELECT amount FROM account_has_asset
                              WHERE asset_id = :asset_id AND
                              account_id = :src_account_id LIMIT 1),
              dest_amount AS (SELECT amount FROM account_has_asset
                               WHERE asset_id = :asset_id AND
                               account_id = :dest_account_id LIMIT 1),
              new_src_value AS (SELECT
                              (SELECT
                                  CASE WHEN EXISTS
                                      (SELECT amount FROM src_amount LIMIT 1)
                                      THEN
                                      (SELECT amount FROM src_amount LIMIT 1)
                                  ELSE 0::decimal
                              END) - :value::decimal AS value
                          ),
              new_dest_value AS (SELECT
                              (SELECT :value::decimal +
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
                      SELECT :src_account_id, :asset_id, value
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
                      SELECT :dest_account_id, :asset_id, value
                      FROM new_dest_value
                      WHERE EXISTS (SELECT * FROM insert_src) AND
                        EXISTS (SELECT * FROM has_src_account LIMIT 1) AND
                        EXISTS (SELECT * FROM has_dest_account LIMIT 1) AND
                        EXISTS (SELECT * FROM has_asset LIMIT 1) AND
                        EXISTS (SELECT value FROM new_dest_value
                                WHERE value < 2::decimal ^ (256 - :precision)
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
                               WHERE value < 2::decimal ^ (256 - :precision)
                               LIMIT 1) THEN 6
              ELSE 7
          END AS result;)");
      if (do_validation_) {
        cmd = (cmd
               % (boost::format(R"(
              has_role_perm AS (%s),
              has_grantable_perm AS (%s),
              dest_can_receive AS (%s),
              has_perm AS (SELECT
                               CASE WHEN (SELECT * FROM dest_can_receive) THEN
                                   CASE WHEN NOT (:creator_id = :src_account_id) THEN
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
                        shared_model::interface::permissions::Role::kTransfer)
                  % checkAccountGrantablePermission(
                        shared_model::interface::permissions::Grantable::
                            kTransferMyAssets)
                  % checkAccountRolePermission(
                        shared_model::interface::permissions::Role::kReceive,
                        "dest_account_id"))
                     .str()
               % R"( AND (SELECT * FROM has_perm))"
               % R"( AND (SELECT * FROM has_perm))"
               % R"( WHEN NOT (SELECT * FROM has_perm) THEN 1 )");
      } else {
        cmd = (cmd % "" % "" % "" % "");
      }
      soci::statement st = sql_.prepare << cmd.str();
      st.exchange(soci::use(src_account_id, "src_account_id"));
      st.exchange(soci::use(dest_account_id, "dest_account_id"));
      st.exchange(soci::use(asset_id, "asset_id"));
      st.exchange(soci::use(amount, "value"));
      st.exchange(soci::use(precision, "precision"));
      st.exchange(soci::use(creator_account_id_, "role_account_id"));
      st.exchange(soci::use(creator_account_id_, "creator_id"));
      st.exchange(soci::use(src_account_id, "grantable_account_id"));
      st.exchange(
          soci::use(creator_account_id_, "grantable_permittee_account_id"));
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
      return makeCommandResultByReturnedValue(st, "TransferAsset", message_gen);
    }
  }  // namespace ametsuchi
}  // namespace iroha
