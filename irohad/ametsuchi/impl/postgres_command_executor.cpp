/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ametsuchi/impl/postgres_command_executor.hpp"

#include <boost/format.hpp>

#include "ametsuchi/impl/postgres_wsv_common.hpp"

namespace iroha {
  namespace ametsuchi {

    PostgresCommandExecutor::PostgresCommandExecutor(soci::session &sql)
        : sql_(sql) {}

    void PostgresCommandExecutor::setCreatorAccountId(
        const shared_model::interface::types::AccountIdType
            &creator_account_id) {
      creator_account_id_ = creator_account_id;
    }

    CommandResult PostgresCommandExecutor::operator()(
        const shared_model::interface::AddAssetQuantity &command) {
      auto &account_id = creator_account_id_;
      auto &asset_id = command.assetId();
      auto amount = command.amount().toStringRepr();
      uint32_t precision = command.amount().precision();
      soci::statement st = sql_.prepare <<
          // clang-format off
                                        (R"(
          WITH has_account AS (SELECT account_id FROM account
                               WHERE account_id = :account_id LIMIT 1),
               has_asset AS (SELECT asset_id FROM asset
                             WHERE asset_id = :asset_id AND
                             precision = :precision LIMIT 1),
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
                                WHERE value < 2 ^ 253 - 1 LIMIT 1)
                  )
                  ON CONFLICT (account_id, asset_id) DO UPDATE
                  SET amount = EXCLUDED.amount
                  RETURNING (1)
               )
          SELECT CASE
              WHEN EXISTS (SELECT * FROM inserted LIMIT 1) THEN 0
              WHEN NOT EXISTS (SELECT * FROM has_account LIMIT 1) THEN 1
              WHEN NOT EXISTS (SELECT * FROM has_asset LIMIT 1) THEN 2
              WHEN NOT EXISTS (SELECT value FROM new_value
                               WHERE value < 2 ^ 253 - 1 LIMIT 1) THEN 3
              ELSE 4
          END AS result;)");
      // clang-format on

      st.exchange(soci::use(account_id, "account_id"));
      st.exchange(soci::use(asset_id, "asset_id"));
      st.exchange(soci::use(amount, "new_value"));
      st.exchange(soci::use(precision, "precision"));

      std::vector<std::function<std::string()>> message_gen = {
          [] { return std::string("Account does not exist"); },
          [] {
            return std::string("Asset with given precision does not exist");
          },
          [] { return std::string("Summation overflows uint256"); },
      };
      return makeCommandResultByValue(st, "AddAssetQuantity", message_gen);
    }

    CommandResult PostgresCommandExecutor::operator()(
        const shared_model::interface::AddPeer &command) {
      auto &peer = command.peer();
      soci::statement st = sql_.prepare
          << "INSERT INTO peer(public_key, address) VALUES (:pk, :address)";
      st.exchange(soci::use(peer.pubkey().hex()));
      st.exchange(soci::use(peer.address()));
      auto message_gen = [&] {
        return (boost::format(
                    "failed to insert peer, public key: '%s', address: '%s'")
                % peer.pubkey().hex() % peer.address())
            .str();
      };
      return makeCommandResult(st, "AddPeer", message_gen);
    }

    CommandResult PostgresCommandExecutor::operator()(
        const shared_model::interface::AddSignatory &command) {
      auto &account_id = command.accountId();
      auto pubkey = command.pubkey().hex();
      soci::statement st = sql_.prepare <<
          R"(
          WITH insert_signatory AS
          (
              INSERT INTO signatory(public_key) VALUES (:pk)
              ON CONFLICT DO NOTHING RETURNING (1)
          ),
          has_signatory AS (SELECT * FROM signatory WHERE public_key = :pk),
          insert_account_signatory AS
          (
              INSERT INTO account_has_signatory(account_id, public_key)
              (
                  SELECT :account_id, :pk WHERE EXISTS
                  (SELECT * FROM insert_signatory) OR
                  EXISTS (SELECT * FROM has_signatory)
              )
              RETURNING (1)
          )
          SELECT CASE
              WHEN EXISTS (SELECT * FROM insert_account_signatory) THEN 0
              WHEN EXISTS (SELECT * FROM insert_signatory) THEN 1
              ELSE 2
          END AS RESULT;)";
      st.exchange(soci::use(pubkey, "pk"));
      st.exchange(soci::use(account_id, "account_id"));

      std::vector<std::function<std::string()>> message_gen = {
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
      return makeCommandResultByValue(st, "AddSignatory", message_gen);
    }

    CommandResult PostgresCommandExecutor::operator()(
        const shared_model::interface::AppendRole &command) {
      auto &account_id = command.accountId();
      auto &role_name = command.roleName();
      soci::statement st = sql_.prepare
          << "INSERT INTO account_has_roles(account_id, role_id) VALUES "
             "(:account_id, :role_id)";
      st.exchange(soci::use(account_id));
      st.exchange(soci::use(role_name));
      auto message_gen = [&] {
        return (boost::format("failed to insert account role, account: '%s', "
                              "role name: '%s'")
                % account_id % role_name)
            .str();
      };
      return makeCommandResult(st, "AppendRole", message_gen);
    }

    CommandResult PostgresCommandExecutor::operator()(
        const shared_model::interface::CreateAccount &command) {
      auto &account_name = command.accountName();
      auto &domain_id = command.domainId();
      auto &pubkey = command.pubkey().hex();
      std::string account_id = account_name + "@" + domain_id;
      soci::statement st = sql_.prepare <<
          R"(
          WITH get_domain_default_role AS (SELECT default_role FROM domain
                                           WHERE domain_id = :domain_id),
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
              WHEN NOT EXISTS (SELECT * FROM account
                               WHERE account_id = :account_id) THEN 1
              WHEN NOT EXISTS (SELECT * FROM account_has_signatory
                               WHERE account_id = :account_id
                               AND public_key = :pk) THEN 2
              WHEN NOT EXISTS (SELECT * FROM account_has_roles
                               WHERE account_id = account_id AND role_id = (
                               SELECT default_role FROM get_domain_default_role)
                               ) THEN 3
              ELSE 4
              END AS result
)";
      st.exchange(soci::use(account_id, "account_id"));
      st.exchange(soci::use(domain_id, "domain_id"));
      st.exchange(soci::use(pubkey, "pk"));
      std::vector<std::function<std::string()>> message_gen = {
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
      return makeCommandResultByValue(st, "CreateAccount", message_gen);
    }

    CommandResult PostgresCommandExecutor::operator()(
        const shared_model::interface::CreateAsset &command) {
      auto &domain_id = command.domainId();
      auto asset_id = command.assetName() + "#" + domain_id;
      auto precision = command.precision();
      soci::statement st = sql_.prepare
          << "INSERT INTO asset(asset_id, domain_id, \"precision\", data) "
             "VALUES (:id, :domain_id, :precision, NULL)";
      st.exchange(soci::use(asset_id));
      st.exchange(soci::use(domain_id));
      st.exchange(soci::use(precision));
      auto message_gen = [&] {
        return (boost::format("failed to insert asset, asset id: '%s', "
                              "domain id: '%s', precision: %d")
                % asset_id % domain_id % precision)
            .str();
      };
      return makeCommandResult(st, "CreateAsset", message_gen);
    }

    CommandResult PostgresCommandExecutor::operator()(
        const shared_model::interface::CreateDomain &command) {
      auto &domain_id = command.domainId();
      auto &default_role = command.userDefaultRole();
      soci::statement st = sql_.prepare
          << "INSERT INTO domain(domain_id, default_role) VALUES (:id, "
             ":role)";
      st.exchange(soci::use(domain_id));
      st.exchange(soci::use(default_role));
      auto message_gen = [&] {
        return (boost::format("failed to insert domain, domain id: '%s', "
                              "default role: '%s'")
                % domain_id % default_role)
            .str();
      };
      return makeCommandResult(st, "CreateDomain", message_gen);
    }

    CommandResult PostgresCommandExecutor::operator()(
        const shared_model::interface::CreateRole &command) {
      auto &role_id = command.roleName();
      auto &permissions = command.rolePermissions();
      auto perm_str = permissions.toBitstring();
      soci::statement st = sql_.prepare <<
          R"(
          WITH insert_role AS (INSERT INTO role(role_id)
                               VALUES (:role_id) RETURNING (1)),
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
              WHEN EXISTS (SELECT * FROM role WHERE role_id = :role_id) THEN 1
              ELSE 2
              END AS result
)";
      st.exchange(soci::use(role_id, "role_id"));
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
            return (boost::format("failed to insert role: '%s'") % role_id)
                .str();
          },
      };
      return makeCommandResultByValue(st, "CreateRole", message_gen);
    }

    CommandResult PostgresCommandExecutor::operator()(
        const shared_model::interface::DetachRole &command) {
      auto &account_id = command.accountId();
      auto &role_name = command.roleName();
      soci::statement st = sql_.prepare
          << "DELETE FROM account_has_roles WHERE account_id=:account_id "
             "AND role_id=:role_id";
      st.exchange(soci::use(account_id));
      st.exchange(soci::use(role_name));
      auto message_gen = [&] {
        return (boost::format(
                    "failed to delete account role, account id: '%s', "
                    "role name: '%s'")
                % account_id % role_name)
            .str();
      };
      return makeCommandResult(st, "DetachRole", message_gen);
    }

    CommandResult PostgresCommandExecutor::operator()(
        const shared_model::interface::GrantPermission &command) {
      auto &permittee_account_id = command.accountId();
      auto &account_id = creator_account_id_;
      auto permission = command.permissionName();
      const auto perm_str =
          shared_model::interface::GrantablePermissionSet({permission})
              .toBitstring();
      soci::statement st = sql_.prepare
          << "INSERT INTO account_has_grantable_permissions as "
             "has_perm(permittee_account_id, account_id, permission) VALUES "
             "(:permittee_account_id, :account_id, :perms) ON CONFLICT "
             "(permittee_account_id, account_id) "
             // SELECT will end up with a error, if the permission exists
             "DO UPDATE SET permission=(SELECT has_perm.permission | :perms "
             "WHERE (has_perm.permission & :perms) <> :perms);";
      st.exchange(soci::use(permittee_account_id, "permittee_account_id"));
      st.exchange(soci::use(account_id, "account_id"));
      st.exchange(soci::use(perm_str, "perms"));
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

      return makeCommandResult(st, "GrantPermission", message_gen);
    }

    CommandResult PostgresCommandExecutor::operator()(
        const shared_model::interface::RemoveSignatory &command) {
      auto &account_id = command.accountId();
      auto &pubkey = command.pubkey().hex();
      soci::statement st = sql_.prepare <<
          R"(
          WITH delete_account_signatory AS (DELETE FROM account_has_signatory
              WHERE account_id = :account_id
              AND public_key = :pk RETURNING (1)),
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
              ELSE 1
          END AS result
)";
      st.exchange(soci::use(account_id, "account_id"));
      st.exchange(soci::use(pubkey, "pk"));
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
      };
      return makeCommandResultByValue(st, "RemoveSignatory", message_gen);
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
      soci::statement st = sql_.prepare
          << "UPDATE account_has_grantable_permissions as has_perm "
             // SELECT will end up with a error, if the permission
             // doesn't exists
             "SET permission=(SELECT has_perm.permission & :without_perm "
             "WHERE has_perm.permission & :perm = :perm AND "
             "has_perm.permittee_account_id=:permittee_account_id AND "
             "has_perm.account_id=:account_id) WHERE "
             "permittee_account_id=:permittee_account_id AND "
             "account_id=:account_id";
      st.exchange(soci::use(permittee_account_id, "permittee_account_id"));
      st.exchange(soci::use(account_id, "account_id"));
      st.exchange(soci::use(without_perm_str, "without_perm"));
      st.exchange(soci::use(perms, "perm"));
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
      return makeCommandResult(st, "RevokePermission", message_gen);
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
      soci::statement st = sql_.prepare
          << "UPDATE account SET data = jsonb_set("
             "CASE WHEN data ?:creator_account_id THEN data ELSE "
             "jsonb_set(data, :json, :empty_json) END, "
             " :filled_json, :val) WHERE account_id=:account_id";
      st.exchange(soci::use(creator_account_id_));
      st.exchange(soci::use(json));
      st.exchange(soci::use(empty_json));
      st.exchange(soci::use(filled_json));
      st.exchange(soci::use(val));
      st.exchange(soci::use(account_id));
      auto message_gen = [&] {
        return (boost::format(
                    "failed to set account key-value, account id: '%s', "
                    "creator account id: '%s',\n key: '%s', value: '%s'")
                % account_id % creator_account_id_ % key % value)
            .str();
      };
      return makeCommandResult(st, "SetAccountDetail", message_gen);
    }

    CommandResult PostgresCommandExecutor::operator()(
        const shared_model::interface::SetQuorum &command) {
      auto &account_id = command.accountId();
      auto quorum = command.newQuorum();
      soci::statement st = sql_.prepare
          << "UPDATE account SET quorum=:quorum WHERE account_id=:account_id";
      st.exchange(soci::use(quorum));
      st.exchange(soci::use(account_id));
      auto message_gen = [&] {
        return (boost::format(
                    "failed to update account, account id: '%s', quorum: '%s'")
                % account_id % quorum)
            .str();
      };
      return makeCommandResult(st, "SetQuorum", message_gen);
    }

    CommandResult PostgresCommandExecutor::operator()(
        const shared_model::interface::SubtractAssetQuantity &command) {
      auto &account_id = creator_account_id_;
      auto &asset_id = command.assetId();
      auto amount = command.amount().toStringRepr();
      uint32_t precision = command.amount().precision();
      soci::statement st = sql_.prepare <<
          // clang-format off
          R"(
          WITH has_account AS (SELECT account_id FROM account
                               WHERE account_id = :account_id LIMIT 1),
               has_asset AS (SELECT asset_id FROM asset
                             WHERE asset_id = :asset_id
                             AND precision = :precision LIMIT 1),
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
                  )
                  ON CONFLICT (account_id, asset_id)
                  DO UPDATE SET amount = EXCLUDED.amount
                  RETURNING (1)
               )
          SELECT CASE
              WHEN EXISTS (SELECT * FROM inserted LIMIT 1) THEN 0
              WHEN NOT EXISTS (SELECT * FROM has_account LIMIT 1) THEN 1
              WHEN NOT EXISTS (SELECT * FROM has_asset LIMIT 1) THEN 2
              WHEN NOT EXISTS
                  (SELECT value FROM new_value WHERE value >= 0 LIMIT 1) THEN 3
              ELSE 4
          END AS result;)";
      // clang-format on
      st.exchange(soci::use(account_id, "account_id"));
      st.exchange(soci::use(asset_id, "asset_id"));
      st.exchange(soci::use(amount, "value"));
      st.exchange(soci::use(precision, "precision"));

      std::vector<std::function<std::string()>> message_gen = {
          [&] { return "Account does not exist"; },
          [&] { return "Asset with given precision does not exist"; },
          [&] { return "Subtracts overdrafts account asset"; },
      };
      return makeCommandResultByValue(st, "SubtractAssetQuantity", message_gen);
    }

    CommandResult PostgresCommandExecutor::operator()(
        const shared_model::interface::TransferAsset &command) {
      auto &src_account_id = command.srcAccountId();
      auto &dest_account_id = command.destAccountId();
      auto &asset_id = command.assetId();
      auto amount = command.amount().toStringRepr();
      uint32_t precision = command.amount().precision();
      soci::statement st = sql_.prepare <<
          // clang-format off
          R"(
          WITH has_src_account AS (SELECT account_id FROM account
                                   WHERE account_id = :src_account_id LIMIT 1),
               has_dest_account AS (SELECT account_id FROM account
                                    WHERE account_id = :dest_account_id
                                    LIMIT 1),
               has_asset AS (SELECT asset_id FROM asset
                             WHERE asset_id = :asset_id LIMIT 1),
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
                                WHERE value >= 0 LIMIT 1)
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
                                WHERE value < 2 ^ 253 - 1 LIMIT 1)
                  )
                  ON CONFLICT (account_id, asset_id)
                  DO UPDATE SET amount = EXCLUDED.amount
                  RETURNING (1)
               )
          SELECT CASE
              WHEN EXISTS (SELECT * FROM insert_dest LIMIT 1) THEN 0
              WHEN NOT EXISTS (SELECT * FROM has_dest_account LIMIT 1) THEN 1
              WHEN NOT EXISTS (SELECT * FROM has_src_account LIMIT 1) THEN 2
              WHEN NOT EXISTS (SELECT * FROM has_asset LIMIT 1) THEN 3
              WHEN NOT EXISTS (SELECT value FROM new_src_value
                               WHERE value >= 0 LIMIT 1) THEN 4
              WHEN NOT EXISTS (SELECT value FROM new_dest_value
                               WHERE value < 2 ^ 253 - 1 LIMIT 1) THEN 5
              ELSE 6
          END AS result;)";
      // clang-format on
      st.exchange(soci::use(src_account_id, "src_account_id"));
      st.exchange(soci::use(dest_account_id, "dest_account_id"));
      st.exchange(soci::use(asset_id, "asset_id"));
      st.exchange(soci::use(amount, "value"));
      st.exchange(soci::use(precision, "precision"));
      std::vector<std::function<std::string()>> message_gen = {
          [&] { return "Destanation account does not exist"; },
          [&] { return "Source account does not exist"; },
          [&] { return "Asset with given precision does not exist"; },
          [&] { return "Transfer overdrafts source account asset"; },
          [&] { return "Transfer overflows destanation account asset"; },
      };
      return makeCommandResultByValue(st, "TransferAsset", message_gen);
    }
  }  // namespace ametsuchi
}  // namespace iroha
