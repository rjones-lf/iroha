/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ametsuchi/impl/postgres_command_executor.hpp"

#include <boost/format.hpp>

namespace iroha {
  namespace ametsuchi {

    PostgresCommandExecutor::PostgresCommandExecutor(
        pqxx::nontransaction &transaction)
        : transaction_(transaction),
          execute_{makeExecuteResult(transaction_)} {}

    void PostgresCommandExecutor::setCreatorAccountId(
        const shared_model::interface::types::AccountIdType
        &creator_account_id) {
        creator_account_id_ = creator_account_id;
    }

    CommandResult PostgresCommandExecutor::operator()(
        const shared_model::interface::AddAssetQuantity &command
        ) {
      auto &account_id = creator_account_id_;
      auto &asset_id = command.assetId();
      auto amount = command.amount().toStringRepr();
      auto precision = command.amount().precision();
      std::string query = (boost::format(
                               // clang-format off
          R"(
          WITH has_account AS (SELECT account_id FROM account WHERE account_id = '%s' LIMIT 1),
               has_asset AS (SELECT asset_id FROM asset WHERE asset_id = '%s' AND precision = %d LIMIT 1),
               amount AS (SELECT amount FROM account_has_asset WHERE asset_id = '%s' AND account_id = '%s' LIMIT 1),
               new_value AS (SELECT %s +
                              (SELECT
                                  CASE WHEN EXISTS (SELECT amount FROM amount LIMIT 1) THEN (SELECT amount FROM amount LIMIT 1)
                                  ELSE 0::decimal
                              END) AS value
                          ),
               inserted AS
               (
                  INSERT INTO account_has_asset(account_id, asset_id, amount)
                  (
                      SELECT '%s', '%s', value FROM new_value
                      WHERE EXISTS (SELECT * FROM has_account LIMIT 1) AND
                        EXISTS (SELECT * FROM has_asset LIMIT 1) AND
                        EXISTS (SELECT value FROM new_value WHERE value < 2 ^ 253 - 1 LIMIT 1)
                  )
                  ON CONFLICT (account_id, asset_id) DO UPDATE SET amount = EXCLUDED.amount
                  RETURNING (1)
               )
          SELECT CASE
              WHEN EXISTS (SELECT * FROM inserted LIMIT 1) THEN 0
              WHEN NOT EXISTS (SELECT * FROM has_account LIMIT 1) THEN 1
              WHEN NOT EXISTS (SELECT * FROM has_asset LIMIT 1) THEN 2
              WHEN NOT EXISTS (SELECT value FROM new_value WHERE value < 2 ^ 253 - 1 LIMIT 1) THEN 3
              ELSE 4
          END AS result;)"
                               // clang-format on
                               )
                           % account_id % asset_id % ((int)precision) % asset_id
                           % account_id % amount % account_id % asset_id)
                              .str();

      auto result = execute_(query);

      return result.match(
          [](expected::Value<pqxx::result> &error_code) -> CommandResult {
            int code = error_code.value[0].at("result").template as<int>();
            if (code == 0) {
              return {};
            }
            std::string error_msg;
            switch (code) {
              case 1:
                error_msg = "Account does not exist";
                break;
              case 2:
                error_msg = "Asset with given precision does not exist";
                break;
              case 3:
                error_msg = "Summation overflows uint256.";
                break;
              default:
                error_msg = "Unexpected error. Something went wrong!";
                break;
            }
            return expected::makeError(error_msg);
          },
          [](const auto &e) -> CommandResult {
            return expected::makeError(e.error);
          });
    }

    CommandResult PostgresCommandExecutor::operator()(
        const shared_model::interface::AddPeer &command) {
      auto &peer = command.peer();
      auto result =
          execute_("INSERT INTO peer(public_key, address) VALUES ("
                   + transaction_.quote(pqxx::binarystring(
                         peer.pubkey().blob().data(), peer.pubkey().size()))
                   + ", " + transaction_.quote(peer.address()) + ");");

      auto message_gen = [&] {
        return (boost::format(
                    "failed to insert peer, public key: '%s', address: '%s'")
                % peer.pubkey().hex() % peer.address())
            .str();
      };
      return makeCommandResult(std::move(result), message_gen);
    }

    CommandResult PostgresCommandExecutor::operator()(
        const shared_model::interface::AddSignatory &command
        ) {
      auto &account_id = command.accountId();
      auto &signatory = command.pubkey();
      auto pubkey = transaction_.quote(
          pqxx::binarystring(signatory.blob().data(), signatory.blob().size()));
      std::string query = (boost::format(
                               R"(
          WITH insert_signatory AS
          (
              INSERT INTO signatory(public_key) VALUES (%s) ON CONFLICT DO NOTHING RETURNING (1)
          ),
          has_signatory AS (SELECT * FROM signatory WHERE public_key = %s),
          insert_account_signatory AS
          (
              INSERT INTO account_has_signatory(account_id, public_key)
              (
                  SELECT '%s', %s WHERE EXISTS (SELECT * FROM insert_signatory) OR EXISTS (SELECT * FROM has_signatory)
              )
              RETURNING (1)
          )
          SELECT CASE
              WHEN EXISTS (SELECT * FROM insert_account_signatory) THEN 0
              WHEN EXISTS (SELECT * FROM insert_signatory) THEN 1
              ELSE 2
          END AS RESULT;)") % pubkey
                           % pubkey % account_id % pubkey)
                              .str();

      auto result = execute_(query);

      return result.match(
          [&signatory, &account_id](
              expected::Value<pqxx::result> &error_code) -> CommandResult {
            int code = error_code.value[0].at("result").template as<int>();
            if (code == 0) {
              return {};
            }
            std::string error_msg;
            switch (code) {
              case 1:
                error_msg =
                    (boost::format(
                         "failed to insert account signatory, account id: "
                         "'%s', signatory hex string: '%s")
                     % account_id % signatory.hex())
                        .str();
                break;
              case 2:
                error_msg = (boost::format("failed to insert signatory, "
                                           "signatory hex string: '%s'")
                             % signatory.hex())
                                .str();
                break;
            }
            return expected::makeError(error_msg);
          },
          [](const auto &e) -> CommandResult {
            return expected::makeError(e.error);
          });
    }

    CommandResult PostgresCommandExecutor::operator()(
        const shared_model::interface::AppendRole &command
        ) {
      auto &account_id = command.accountId();
      auto &role_name = command.roleName();
      auto result =
          execute_("INSERT INTO account_has_roles(account_id, role_id) VALUES ("
                   + transaction_.quote(account_id) + ", "
                   + transaction_.quote(role_name) + ");");

      auto message_gen = [&] {
        return (boost::format("failed to insert account role, account: '%s', "
                              "role name: '%s'")
                % account_id % role_name)
            .str();
      };
      return makeCommandResult(std::move(result), message_gen);
    }

    CommandResult PostgresCommandExecutor::operator()(
        const shared_model::interface::CreateAccount &command) {
      auto &account_name = command.accountName();
      auto &domain_id = command.domainId();
      auto &pubkey = command.pubkey();
      std::string account_id = account_name + "@" + domain_id;
      auto pk = transaction_.quote(
          pqxx::binarystring(pubkey.blob().data(), pubkey.blob().size()));
      std::string query = (boost::format(
                               R"(
          WITH get_domain_default_role AS (SELECT default_role FROM domain WHERE domain_id = '%s'),
          insert_signatory AS
          (
              INSERT INTO signatory(public_key)
              (
                  SELECT %s WHERE EXISTS (SELECT * FROM get_domain_default_role)
              ) ON CONFLICT DO NOTHING RETURNING (1)
          ),
          has_signatory AS (SELECT * FROM signatory WHERE public_key = %s),
          insert_account AS
          (
              INSERT INTO account(account_id, domain_id, quorum, data)
              (
                  SELECT '%s', '%s', 1, '{}' WHERE (EXISTS (SELECT * FROM insert_signatory) OR EXISTS (SELECT * FROM has_signatory)) AND EXISTS (SELECT * FROM get_domain_default_role)
              ) RETURNING (1)
          ),
          insert_account_signatory AS
          (
              INSERT INTO account_has_signatory(account_id, public_key)
              (
                  SELECT '%s', %s WHERE
                     EXISTS (SELECT * FROM insert_account)
              )
              RETURNING (1)
          ),
          insert_account_role AS
          (
              INSERT INTO account_has_roles(account_id, role_id)
              (
                  SELECT '%s', default_role FROM get_domain_default_role
                  WHERE EXISTS (SELECT * FROM get_domain_default_role)
                    AND EXISTS (SELECT * FROM insert_account_signatory)
              ) RETURNING (1)
          )
          SELECT CASE
              WHEN EXISTS (SELECT * FROM insert_account_role) THEN 0
              WHEN NOT EXISTS (SELECT * FROM account WHERE account_id = '%s') THEN 1
              WHEN NOT EXISTS (SELECT * FROM account_has_signatory WHERE account_id = '%s' AND public_key = %s) THEN 2
              WHEN NOT EXISTS (SELECT * FROM account_has_roles WHERE account_id = '%s' AND role_id = (SELECT default_role FROM get_domain_default_role)) THEN 3
              ELSE 4
              END AS result
)") % domain_id % pk % pk % account_id
                           % domain_id % account_id % pk % account_id
                           % account_id % account_id % pk % account_id)
                              .str();
      auto result = execute_(query);

      return result.match(
          [&](expected::Value<pqxx::result> &error_code) -> CommandResult {
            int code = error_code.value[0].at("result").template as<int>();
            if (code == 0) {
              return {};
            }
            std::string error_msg;
            switch (code) {
              case 1:
                error_msg = (boost::format("failed to insert account, "
                                           "account id: '%s', "
                                           "domain id: '%s', "
                                           "quorum: '1', "
                                           "json_data: {}")
                             % account_id % domain_id)
                                .str();
                break;
              case 2:
                error_msg =
                    (boost::format(
                         "failed to insert account signatory, account id: "
                         "'%s', signatory hex string: '%s")
                     % account_id % pubkey.hex())
                        .str();
                break;
              case 3:
                error_msg =
                    (boost::format(
                         "failed to insert account role, account: '%s' "
                         "with default domain role name for domain: '%s'")
                     % account_id % domain_id)
                        .str();
                break;
              default:
                error_msg = "Unexpected error";
                break;
            }
            return expected::makeError(error_msg);
          },
          [](const auto &e) -> CommandResult {
            return expected::makeError(e.error);
          });
    }

    CommandResult PostgresCommandExecutor::operator()(
        const shared_model::interface::CreateAsset &command) {

      auto &asset_name = command.assetName();
      auto &domain_id = command.domainId();
      auto precision = command.precision();
      shared_model::interface::types::AssetIdType asset_id =
          asset_name + "#" + domain_id;
      auto result = execute_(
          "INSERT INTO asset(asset_id, domain_id, \"precision\", data) "
          "VALUES ("
          + transaction_.quote(asset_id) + ", " + transaction_.quote(domain_id)
          + ", " + transaction_.quote((uint32_t)precision) + ", NULL" + ");");

      auto message_gen = [&] {
        return (boost::format("failed to insert asset, asset id: '%s', "
                              "domain id: '%s', precision: %d")
                % asset_id % domain_id % precision)
            .str();
      };

      return makeCommandResult(std::move(result), message_gen);
    }

    CommandResult PostgresCommandExecutor::operator()(
        const shared_model::interface::CreateDomain &command) {
      auto &domain_id = command.domainId();
      auto &default_role = command.userDefaultRole();
      auto result =
          execute_("INSERT INTO domain(domain_id, default_role) VALUES ("
                   + transaction_.quote(domain_id) + ", "
                   + transaction_.quote(default_role) + ");");

      auto message_gen = [&] {
        return (boost::format("failed to insert domain, domain id: '%s', "
                              "default role: '%s'")
                % domain_id % default_role)
            .str();
      };
      return makeCommandResult(std::move(result), message_gen);
    }

    CommandResult PostgresCommandExecutor::operator()(
        const shared_model::interface::CreateRole &command) {
      auto &role_id = command.roleName();
      auto &permissions = command.rolePermissions();
      auto perm_str = permissions.toBitstring();
      std::string query = (boost::format(
                               R"(
          WITH insert_role AS (INSERT INTO role(role_id) VALUES ('%s') RETURNING (1)),
          insert_role_permissions AS
          (
              INSERT INTO role_has_permissions(role_id, permission)
              (
                  SELECT '%s', '%s' WHERE EXISTS (SELECT * FROM insert_role)
              ) RETURNING (1)
          )
          SELECT CASE
              WHEN EXISTS (SELECT * FROM insert_role_permissions) THEN 0
              WHEN EXISTS (SELECT * FROM role WHERE role_id = '%s') THEN 1
              ELSE 2
              END AS result
)") % role_id % role_id % perm_str
                           % role_id)
                              .str();

      auto result = execute_(query);

      return result.match(
          [&](expected::Value<pqxx::result> &error_code) -> CommandResult {
            int code = error_code.value[0].at("result").template as<int>();
            if (code == 0) {
              return {};
            }
            std::string error_msg;
            // TODO(@l4l) 26/06/18 need to be simplified at IR-1479
            const auto &str =
                shared_model::proto::permissions::toString(permissions);
            const auto perm_debug_str =
                std::accumulate(str.begin(), str.end(), std::string());
            switch (code) {
              case 1:
                error_msg =
                    (boost::format("failed to insert role permissions, role "
                                   "id: '%s', permissions: [%s]")
                     % role_id % perm_debug_str)
                        .str();
                break;
              case 2:
                error_msg =
                    (boost::format("failed to insert role: '%s'") % role_id)
                        .str();
                break;
              default:
                error_msg = "Unexpected error";
                break;
            }
            return expected::makeError(error_msg);
          },
          [](const auto &e) -> CommandResult {
            return expected::makeError(e.error);
          });
    }

    CommandResult PostgresCommandExecutor::operator()(
        const shared_model::interface::DetachRole &command
        ) {
      auto &account_id = command.accountId();
      auto &role_name = command.roleName();
      auto result = execute_("DELETE FROM account_has_roles WHERE account_id="
                             + transaction_.quote(account_id) + "AND role_id="
                             + transaction_.quote(role_name) + ";");
      auto message_gen = [&] {
        return (boost::format(
                    "failed to delete account role, account id: '%s', "
                    "role name: '%s'")
                % account_id % role_name)
            .str();
      };

      return makeCommandResult(std::move(result), message_gen);
    }

    CommandResult PostgresCommandExecutor::operator()(
        const shared_model::interface::GrantPermission &command
        ) {
      auto
          &permittee_account_id = command.accountId();
      auto &account_id = creator_account_id_;
      auto permission = command.permissionName();
      const auto perm_str =
          shared_model::interface::GrantablePermissionSet({permission})
              .toBitstring();
      auto query =
          (boost::format(
               "INSERT INTO account_has_grantable_permissions as "
               "has_perm(permittee_account_id, account_id, permission) VALUES "
               "(%1%, %2%, %3%) ON CONFLICT (permittee_account_id, account_id) "
               // SELECT will end up with a error, if the permission exists
               "DO UPDATE SET permission=(SELECT has_perm.permission | %3% "
               "WHERE (has_perm.permission & %3%) <> %3%);")
           % transaction_.quote(permittee_account_id)
           % transaction_.quote(account_id) % transaction_.quote(perm_str))
              .str();
      auto result = execute_(query);

      auto message_gen = [&] {
        return (boost::format("failed to insert account grantable permission, "
                              "permittee account id: '%s', "
                              "account id: '%s', "
                              "permission: '%s'")
                % permittee_account_id
                % account_id
                // TODO(@l4l) 26/06/18 need to be simplified at IR-1479
                % shared_model::proto::permissions::toString(permission))
            .str();
      };

      return makeCommandResult(std::move(result), message_gen);
    }

    CommandResult PostgresCommandExecutor::operator()(
        const shared_model::interface::RemoveSignatory &command) {
      auto &account_id = command.accountId();
      auto &pubkey = command.pubkey();
      auto pk = transaction_.quote(
          pqxx::binarystring(pubkey.blob().data(), pubkey.blob().size()));
      std::string query = (boost::format(
                               R"(
          WITH delete_account_signatory AS (DELETE FROM account_has_signatory
              WHERE account_id = '%s' AND public_key = %s RETURNING (1)),
          delete_signatory AS
          (
              DELETE FROM signatory WHERE public_key = %s AND
                  NOT EXISTS (SELECT 1 FROM account_has_signatory WHERE public_key = %s)
                  AND NOT EXISTS (SELECT 1 FROM peer WHERE public_key = %s)
              RETURNING (1)
          )
          SELECT CASE
              WHEN EXISTS (SELECT * FROM delete_account_signatory) THEN
              CASE
                  WHEN EXISTS (SELECT * FROM delete_signatory) THEN 0
                  WHEN EXISTS (SELECT 1 FROM account_has_signatory WHERE public_key = %s) THEN 0
                  WHEN EXISTS (SELECT 1 FROM peer WHERE public_key = %s) THEN 0
                  ELSE 2
              END
              ELSE 1
          END AS result
)") % account_id % pk % pk % pk
                           % pk % pk % pk)
                              .str();

      auto result = execute_(query);

      return result.match(
          [&](expected::Value<pqxx::result> &error_code) -> CommandResult {
            int code = error_code.value[0].at("result").template as<int>();
            if (code == 0) {
              return {};
            }
            std::string error_msg;
            switch (code) {
              case 1:
                error_msg =
                    (boost::format(
                         "failed to delete account signatory, account id: "
                         "'%s', signatory hex string: '%s'")
                     % account_id % pubkey.hex())
                        .str();
                break;
              case 2:
                error_msg = (boost::format("failed to delete signatory, "
                                           "signatory hex string: '%s'")
                             % pubkey.hex())
                                .str();
                break;
              default:
                error_msg = "Unexpected error";
                break;
            }
            return expected::makeError(error_msg);
          },
          [](const auto &e) -> CommandResult {
            return expected::makeError(e.error);
          });
    }

    CommandResult PostgresCommandExecutor::operator()(
        const shared_model::interface::RevokePermission &command) {
      auto
          &permittee_account_id = command.accountId();
      auto &account_id = creator_account_id_;
      auto permission = command.permissionName();
      const auto without_perm_str = shared_model::interface::GrantablePermissionSet()
                                .set()
                                .unset(permission)
                                .toBitstring();
      const auto with_perm_str = shared_model::interface::GrantablePermissionSet()
          .set(permission)
          .toBitstring();
      auto query =
          (boost::format("UPDATE account_has_grantable_permissions as has_perm "
                         // SELECT will end up with a error, if the permission
                         // doesn't exists
                         "SET permission=(SELECT has_perm.permission & %3% "
                         "WHERE has_perm.permission & %4% = %4% AND "
                         "has_perm.permittee_account_id=%1% AND has_perm.account_id=%2%) WHERE "
                         "permittee_account_id=%1% AND account_id=%2%;")
           % transaction_.quote(permittee_account_id)
           % transaction_.quote(account_id) % transaction_.quote(without_perm_str)
           % transaction_.quote(with_perm_str))
              .str();
      auto result = execute_(query);

      auto message_gen = [&] {
        return (boost::format("failed to delete account grantable permission, "
                              "permittee account id: '%s', "
                              "account id: '%s', "
                              "permission id: '%s'")
                % permittee_account_id
                % account_id
                // TODO(@l4l) 26/06/18 need to be simplified at IR-1479
                % shared_model::proto::permissions::toString(permission))
            .str();
      };

      return makeCommandResult(std::move(result), message_gen);
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
      auto result = execute_(
          "UPDATE account SET data = jsonb_set(CASE WHEN data ?"
          + transaction_.quote(creator_account_id_) + " THEN data ELSE jsonb_set(data, "
          + transaction_.quote("{" + creator_account_id_ + "}") + "," + transaction_.quote("{}")
          + ") END," + transaction_.quote("{" + creator_account_id_ + ", " + key + "}") + ","
          + transaction_.quote("\"" + value + "\"")
          + ") WHERE account_id=" + transaction_.quote(account_id) + ";");

      auto message_gen = [&] {
        return (boost::format(
                    "failed to set account key-value, account id: '%s', "
                    "creator account id: '%s',\n key: '%s', value: '%s'")
                % account_id % creator_account_id_ % key % value)
            .str();
      };
      return makeCommandResult(std::move(result), message_gen);
    }

    CommandResult PostgresCommandExecutor::operator()(
        const shared_model::interface::SetQuorum &command) {
      auto &account_id = command.accountId();
      auto quorum = command.newQuorum();
      auto result = execute_(
          "UPDATE account\n"
              "   SET quorum=" +
              transaction_.quote(quorum) +
              "\n"
                  " WHERE account_id=" +
              transaction_.quote(account_id) + "AND "
              + transaction_.quote(quorum) + " BETWEEN 1 AND 128");

      auto message_gen = [&] {
        return (boost::format(
                    "failed to update account, account id: '%s', quorum: '%s'")
                % account_id % quorum)
            .str();
      };
      return makeCommandResult(std::move(result), message_gen);
    }

    CommandResult PostgresCommandExecutor::operator()(
        const shared_model::interface::SubtractAssetQuantity &command) {

      auto &account_id = creator_account_id_;
      auto &asset_id = command.assetId();
      auto amount = command.amount().toStringRepr();
      auto precision = command.amount().precision();
      std::string query = (boost::format(
                               // clang-format off
          R"(
          WITH has_account AS (SELECT account_id FROM account WHERE account_id = '%s' LIMIT 1),
               has_asset AS (SELECT asset_id FROM asset WHERE asset_id = '%s' AND precision = %d LIMIT 1),
               amount AS (SELECT amount FROM account_has_asset WHERE asset_id = '%s' AND account_id = '%s' LIMIT 1),
               new_value AS (SELECT
                              (SELECT
                                  CASE WHEN EXISTS (SELECT amount FROM amount LIMIT 1) THEN (SELECT amount FROM amount LIMIT 1)
                                  ELSE 0::decimal
                              END) - %s AS value
                          ),
               inserted AS
               (
                  INSERT INTO account_has_asset(account_id, asset_id, amount)
                  (
                      SELECT '%s', '%s', value FROM new_value
                      WHERE EXISTS (SELECT * FROM has_account LIMIT 1) AND
                        EXISTS (SELECT * FROM has_asset LIMIT 1) AND
                        EXISTS (SELECT value FROM new_value WHERE value >= 0 LIMIT 1)
                  )
                  ON CONFLICT (account_id, asset_id) DO UPDATE SET amount = EXCLUDED.amount
                  RETURNING (1)
               )
          SELECT CASE
              WHEN EXISTS (SELECT * FROM inserted LIMIT 1) THEN 0
              WHEN NOT EXISTS (SELECT * FROM has_account LIMIT 1) THEN 1
              WHEN NOT EXISTS (SELECT * FROM has_asset LIMIT 1) THEN 2
              WHEN NOT EXISTS (SELECT value FROM new_value WHERE value >= 0 LIMIT 1) THEN 3
              ELSE 4
          END AS result;)"
                               // clang-format on
                               )
                           % account_id % asset_id % ((int)precision) % asset_id
                           % account_id % amount % account_id % asset_id)
                              .str();

      auto result = execute_(query);

      return result.match(
          [](expected::Value<pqxx::result> &error_code) -> CommandResult {
            int code = error_code.value[0].at("result").template as<int>();
            if (code == 0) {
              return {};
            }
            std::string error_msg;
            switch (code) {
              case 1:
                error_msg = "Account does not exist";
                break;
              case 2:
                error_msg = "Asset with given precision does not exist";
                break;
              case 3:
                error_msg = "Subtracts more than have.";
                break;
              default:
                error_msg = "Unexpected error. Something went wrong!";
                break;
            }
            return expected::makeError(error_msg);
          },
          [](const auto &e) -> CommandResult {
            return expected::makeError(e.error);
          });
    }

    CommandResult PostgresCommandExecutor::operator()(
        const shared_model::interface::TransferAsset &command) {

      auto &src_account_id = command.srcAccountId();
      auto &dest_account_id = command.destAccountId();
      auto &asset_id = command.assetId();
      auto amount = command.amount().toStringRepr();
      auto precision = command.amount().precision();
      std::string query =
          (boost::format(
               // clang-format off
          R"(
          WITH has_src_account AS (SELECT account_id FROM account WHERE account_id = '%s' LIMIT 1),
               has_dest_account AS (SELECT account_id FROM account WHERE account_id = '%s' LIMIT 1),
               has_asset AS (SELECT asset_id FROM asset WHERE asset_id = '%s' LIMIT 1),
               src_amount AS (SELECT amount FROM account_has_asset WHERE asset_id = '%s' AND account_id = '%s' LIMIT 1),
               dest_amount AS (SELECT amount FROM account_has_asset WHERE asset_id = '%s' AND account_id = '%s' LIMIT 1),
               new_src_value AS (SELECT
                              (SELECT
                                  CASE WHEN EXISTS (SELECT amount FROM src_amount LIMIT 1) THEN
                                      (SELECT amount FROM src_amount LIMIT 1)
                                  ELSE 0::decimal
                              END) - %s AS value
                          ),
               new_dest_value AS (SELECT
                              (SELECT %s +
                                  CASE WHEN EXISTS (SELECT amount FROM dest_amount LIMIT 1) THEN
                                      (SELECT amount FROM dest_amount LIMIT 1)
                                  ELSE 0::decimal
                              END) AS value
                          ),
               insert_src AS
               (
                  INSERT INTO account_has_asset(account_id, asset_id, amount)
                  (
                      SELECT '%s', '%s', value FROM new_src_value
                      WHERE EXISTS (SELECT * FROM has_src_account LIMIT 1) AND
                        EXISTS (SELECT * FROM has_dest_account LIMIT 1) AND
                        EXISTS (SELECT * FROM has_asset LIMIT 1) AND
                        EXISTS (SELECT value FROM new_src_value WHERE value >= 0 LIMIT 1)
                  )
                  ON CONFLICT (account_id, asset_id) DO UPDATE SET amount = EXCLUDED.amount
                  RETURNING (1)
               ),
               insert_dest AS
               (
                  INSERT INTO account_has_asset(account_id, asset_id, amount)
                  (
                      SELECT '%s', '%s', value FROM new_dest_value
                      WHERE EXISTS (SELECT * FROM insert_src) AND
                        EXISTS (SELECT * FROM has_src_account LIMIT 1) AND
                        EXISTS (SELECT * FROM has_dest_account LIMIT 1) AND
                        EXISTS (SELECT * FROM has_asset LIMIT 1) AND
                        EXISTS (SELECT value FROM new_dest_value WHERE value < 2 ^ 253 - 1 LIMIT 1)
                  )
                  ON CONFLICT (account_id, asset_id) DO UPDATE SET amount = EXCLUDED.amount
                  RETURNING (1)
               )
          SELECT CASE
              WHEN EXISTS (SELECT * FROM insert_dest LIMIT 1) THEN 0
              WHEN NOT EXISTS (SELECT * FROM has_dest_account LIMIT 1) THEN 1
              WHEN NOT EXISTS (SELECT * FROM has_src_account LIMIT 1) THEN 2
              WHEN NOT EXISTS (SELECT * FROM has_asset LIMIT 1) THEN 3
              WHEN NOT EXISTS (SELECT value FROM new_src_value WHERE value >= 0 LIMIT 1) THEN 4
              WHEN NOT EXISTS (SELECT value FROM new_dest_value WHERE value < 2 ^ 253 - 1 LIMIT 1) THEN 5
              ELSE 6
          END AS result;)"
               // clang-format on
               )
           % src_account_id % dest_account_id % asset_id % asset_id
           % src_account_id % asset_id % dest_account_id % amount % amount
           % src_account_id % asset_id % dest_account_id % asset_id)
              .str();

      auto result = execute_(query);

      return result.match(
          [](expected::Value<pqxx::result> &error_code) -> CommandResult {
            int code = error_code.value[0].at("result").template as<int>();
            if (code == 0) {
              return {};
            }
            std::string error_msg;
            switch (code) {
              case 1:
                error_msg = "Destanation account does not exist";
                break;
              case 2:
                error_msg = "Source account does not exist";
                break;
              case 3:
                error_msg = "Asset with given precision does not exist";
                break;
              case 4:
                error_msg = "Transfer overdrafts source account asset";
                break;
              case 5:
                error_msg = "Transfer overflows destanation account asset";
                break;
              default:
                error_msg = "Unexpected error. Something went wrong!";
                break;
            }
            return expected::makeError(error_msg);
          },
          [](const auto &e) -> CommandResult {
            return expected::makeError(e.error);
          });
    }
  }  // namespace ametsuchi
}  // namespace iroha
