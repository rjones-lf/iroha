/**
 * Copyright Soramitsu Co., Ltd. 2018 All Rights Reserved.
 * http://soramitsu.co.jp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ametsuchi/impl/postgres_wsv_command.hpp"

#include <boost/format.hpp>

#include "model/account.hpp"
#include "model/account_asset.hpp"
#include "model/asset.hpp"
#include "model/domain.hpp"
#include "model/peer.hpp"

namespace iroha {
  namespace ametsuchi {

    PostgresWsvCommand::PostgresWsvCommand(pqxx::nontransaction &transaction)
        : transaction_(transaction),
          execute_{makeExecuteResult(transaction_)} {}

    WsvCommandResult PostgresWsvCommand::insertRole(
        const shared_model::interface::types::RoleIdType &role_name) {
      auto result = execute_("INSERT INTO role(role_id) VALUES ("
                             + transaction_.quote(role_name) + ");");

      auto message_gen = [&] {
        return (boost::format("failed to insert role: '%s'") % role_name).str();
      };

      return makeCommandResult(std::move(result), message_gen);
    }

    WsvCommandResult PostgresWsvCommand::insertAccountRole(
        const shared_model::interface::types::AccountIdType &account_id,
        const shared_model::interface::types::RoleIdType &role_name) {
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

    WsvCommandResult PostgresWsvCommand::deleteAccountRole(
        const shared_model::interface::types::AccountIdType &account_id,
        const shared_model::interface::types::RoleIdType &role_name) {
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

    WsvCommandResult PostgresWsvCommand::insertRolePermissions(
        const shared_model::interface::types::RoleIdType &role_id,
        const std::set<shared_model::interface::types::PermissionNameType>
            &permissions) {
      auto entry = [this, &role_id](auto permission) {
        return "(" + transaction_.quote(role_id) + ", "
            + transaction_.quote(permission) + ")";
      };

      // generate string with all permissions,
      // applying transform_func to each permission
      auto generate_perm_string = [&permissions](auto transform_func) {
        return std::accumulate(std::next(permissions.begin()),
                               permissions.end(),
                               transform_func(*permissions.begin()),
                               [&transform_func](auto &res, auto &perm) {
                                 return res + ", " + transform_func(perm);
                               });
      };

      auto result = execute_(
          "INSERT INTO role_has_permissions(role_id, permission_id) VALUES "
          + generate_perm_string(entry) + ";");

      auto message_gen = [&] {
        return (boost::format("failed to insert role permissions, role "
                              "id: '%s', permissions: [%s]")
                % role_id % generate_perm_string([](auto &a) { return a; }))
            .str();
      };

      return makeCommandResult(std::move(result), message_gen);
    }

    WsvCommandResult PostgresWsvCommand::insertAccountGrantablePermission(
        const shared_model::interface::types::AccountIdType
            &permittee_account_id,
        const shared_model::interface::types::AccountIdType &account_id,
        const shared_model::interface::types::PermissionNameType
            &permission_id) {
      auto result = execute_(
          "INSERT INTO "
          "account_has_grantable_permissions(permittee_account_id, "
          "account_id, permission_id) VALUES ("
          + transaction_.quote(permittee_account_id) + ", "
          + transaction_.quote(account_id) + ", "
          + transaction_.quote(permission_id) + ");");

      auto message_gen = [&] {
        return (boost::format("failed to insert account grantable permission, "
                              "permittee account id: '%s', "
                              "account id: '%s', "
                              "permission id: '%s'")
                % permittee_account_id % account_id % permission_id)
            .str();
      };

      return makeCommandResult(std::move(result), message_gen);
    }

    WsvCommandResult PostgresWsvCommand::deleteAccountGrantablePermission(
        const shared_model::interface::types::AccountIdType
            &permittee_account_id,
        const shared_model::interface::types::AccountIdType &account_id,
        const shared_model::interface::types::PermissionNameType
            &permission_id) {
      auto result = execute_(
          "DELETE FROM public.account_has_grantable_permissions WHERE "
          "permittee_account_id="
          + transaction_.quote(permittee_account_id)
          + " AND account_id=" + transaction_.quote(account_id)
          + " AND permission_id=" + transaction_.quote(permission_id) + " ;");

      auto message_gen = [&] {
        return (boost::format("failed to delete account grantable permission, "
                              "permittee account id: '%s', "
                              "account id: '%s', "
                              "permission id: '%s'")
                % permittee_account_id % account_id % permission_id)
            .str();
      };

      return makeCommandResult(std::move(result), message_gen);
    }

    WsvCommandResult PostgresWsvCommand::insertAccount(
        const shared_model::interface::Account &account) {
      auto result = execute_(
          "INSERT INTO account(account_id, domain_id, quorum, "
          "transaction_count, data) VALUES ("
          + transaction_.quote(account.accountId()) + ", "
          + transaction_.quote(account.domainId()) + ", "
          + transaction_.quote(account.quorum())
          + ", "
          // Transaction counter
          + transaction_.quote(default_tx_counter) + ", "
          + transaction_.quote(account.jsonData()) + ");");

      auto message_gen = [&] {
        return (boost::format("failed to insert account, "
                              "account id: '%s', "
                              "domain id: '%s', "
                              "quorum: '%d', "
                              "transaction counter: '%d', "
                              "json_data: %s")
                % account.accountId() % account.domainId() % account.quorum()
                % default_tx_counter % account.jsonData())
            .str();
      };

      return makeCommandResult(std::move(result), message_gen);
    }

    WsvCommandResult PostgresWsvCommand::insertAsset(
        const shared_model::interface::Asset &asset) {
      uint32_t precision = asset.precision();
      auto result = execute_(
          "INSERT INTO asset(asset_id, domain_id, \"precision\", data) "
          "VALUES ("
          + transaction_.quote(asset.assetId()) + ", "
          + transaction_.quote(asset.domainId()) + ", "
          + transaction_.quote(precision) + ", " + /*asset.data*/ "NULL"
          + ");");

      auto message_gen = [&] {
        return (boost::format("failed to insert asset, asset id: '%s', "
                              "domain id: '%s', precision: %d")
                % asset.assetId() % asset.domainId() % precision)
            .str();
      };

      return makeCommandResult(std::move(result), message_gen);
    }

    WsvCommandResult PostgresWsvCommand::upsertAccountAsset(
        const shared_model::interface::AccountAsset &asset) {
      auto result = execute_(
            "INSERT INTO account_has_asset(account_id, asset_id, amount) "
            "VALUES ("
            + transaction_.quote(asset.accountId()) + ", "
            + transaction_.quote(asset.assetId()) + ", "
            + transaction_.quote(asset.balance().toStringRepr())
            + ") ON CONFLICT (account_id, asset_id) DO UPDATE SET "
            "amount = EXCLUDED.amount;");

      auto message_gen = [&] {
        return (boost::format("failed to upsert account, account id: '%s', "
                              "asset id: '%s', balance: %s")
                % asset.accountId() % asset.assetId()
                % asset.balance().toString())
            .str();
      };

      return makeCommandResult(std::move(result), message_gen);
    }

    WsvCommandResult PostgresWsvCommand::insertSignatory(
        const shared_model::interface::types::PubkeyType &signatory) {
      auto result =
          execute_("INSERT INTO signatory(public_key) VALUES ("
                   + transaction_.quote(pqxx::binarystring(
                         signatory.blob().data(), signatory.blob().size()))
                   + ") ON CONFLICT DO NOTHING;");
      auto message_gen = [&] {
        return (boost::format(
                    "failed to insert signatory, signatory hex string: '%s'")
                % signatory.hex())
            .str();
      };
      return makeCommandResult(std::move(result), message_gen);
    }

    WsvCommandResult PostgresWsvCommand::insertAccountSignatory(
        const shared_model::interface::types::AccountIdType &account_id,
        const shared_model::interface::types::PubkeyType &signatory) {
      auto result = execute_(
          "INSERT INTO account_has_signatory(account_id, public_key) VALUES ("
          + transaction_.quote(account_id) + ", "
          + transaction_.quote(pqxx::binarystring(signatory.blob().data(),
                                                  signatory.blob().size()))
          + ");");

      auto message_gen = [&] {
        return (boost::format("failed to insert account signatory, account id: "
                              "'%s', signatory hex string: '%s")
                % account_id % signatory.hex())
            .str();
      };
      return makeCommandResult(std::move(result), message_gen);
    }

    WsvCommandResult PostgresWsvCommand::deleteAccountSignatory(
        const shared_model::interface::types::AccountIdType &account_id,
        const shared_model::interface::types::PubkeyType &signatory) {
      auto result =
          execute_("DELETE FROM account_has_signatory WHERE account_id = "
                   + transaction_.quote(account_id) + " AND public_key = "
                   + transaction_.quote(pqxx::binarystring(
                         signatory.blob().data(), signatory.blob().size()))
                   + ";");

      auto message_gen = [&] {
        return (boost::format("failed to delete account signatory, account id: "
                              "'%s', signatory hex string: '%s'")
                % account_id % signatory.hex())
            .str();
      };
      return makeCommandResult(std::move(result), message_gen);
    }

    WsvCommandResult PostgresWsvCommand::deleteSignatory(
        const shared_model::interface::types::PubkeyType &signatory) {
      pqxx::binarystring public_key(signatory.blob().data(),
                                    signatory.blob().size());
      auto result = execute_("DELETE FROM signatory WHERE public_key = "
                    + transaction_.quote(public_key)
                    + " AND NOT EXISTS (SELECT 1 FROM account_has_signatory "
                        "WHERE public_key = "
                    + transaction_.quote(public_key)
                    + ") AND NOT EXISTS (SELECT 1 FROM peer WHERE public_key = "
                    + transaction_.quote(public_key) + ");");

      auto message_gen = [&] {
        return (boost::format(
                    "failed to delete signatory, signatory hex string: '%s'")
                % signatory.hex())
            .str();
      };
      return makeCommandResult(std::move(result), message_gen);
    }

    WsvCommandResult PostgresWsvCommand::insertPeer(
        const shared_model::interface::Peer &peer) {
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

    WsvCommandResult PostgresWsvCommand::deletePeer(
        const shared_model::interface::Peer &peer) {
      auto result = execute_(
          "DELETE FROM peer WHERE public_key = "
          + transaction_.quote(pqxx::binarystring(peer.pubkey().blob().data(),
                                                  peer.pubkey().size()))
          + " AND address = " + transaction_.quote(peer.address()) + ";");
      auto message_gen = [&] {
        return (boost::format(
                    "failed to delete peer, public key: '%s', address: '%s'")
                % peer.pubkey().hex() % peer.address())
            .str();
      };
      return makeCommandResult(std::move(result), message_gen);
    }

    WsvCommandResult PostgresWsvCommand::insertDomain(
        const shared_model::interface::Domain &domain) {
      auto result =
          execute_("INSERT INTO domain(domain_id, default_role) VALUES ("
                   + transaction_.quote(domain.domainId()) + ", "
                   + transaction_.quote(domain.defaultRole()) + ");");

      auto message_gen = [&] {
        return (boost::format("failed to insert domain, domain id: '%s', "
                              "default role: '%s'")
                % domain.domainId() % domain.defaultRole())
            .str();
      };
      return makeCommandResult(std::move(result), message_gen);
    }

    WsvCommandResult PostgresWsvCommand::updateAccount(
        const shared_model::interface::Account &account) {
      auto result = execute_(
            "UPDATE account\n"
            "   SET quorum=" +
            transaction_.quote(account.quorum()) +
            ", transaction_count=" +
            /*account.transaction_count*/ transaction_.quote(default_tx_counter) +
            "\n"
            " WHERE account_id=" +
            transaction_.quote(account.accountId()) + ";");

      auto message_gen = [&] {
        return (boost::format(
                    "failed to update account, account id: '%s', quorum: '%s'")
                % account.accountId() % account.quorum())
            .str();
      };
      return makeCommandResult(std::move(result), message_gen);
    }

    WsvCommandResult PostgresWsvCommand::setAccountKV(
        const shared_model::interface::types::AccountIdType &account_id,
        const shared_model::interface::types::AccountIdType &creator_account_id,
        const std::string &key,
        const std::string &val) {
      auto result = execute_(
          "UPDATE account SET data = jsonb_set(CASE WHEN data ?"
          + transaction_.quote(creator_account_id)
          + " THEN data ELSE jsonb_set(data, "
          + transaction_.quote("{" + creator_account_id + "}") + ","
          + transaction_.quote("{}") + ") END,"
          + transaction_.quote("{" + creator_account_id + ", " + key + "}")
          + "," + transaction_.quote("\"" + val + "\"")
          + ") WHERE account_id=" + transaction_.quote(account_id) + ";");

      auto message_gen = [&] {
        return (boost::format(
                    "failed to set account key-value, account id: '%s', "
                    "creator account id: '%s',\n key: '%s', value: '%s'")
                % account_id % creator_account_id % key % val)
            .str();
      };
      return makeCommandResult(std::move(result), message_gen);
    }
  }  // namespace ametsuchi
}  // namespace iroha
