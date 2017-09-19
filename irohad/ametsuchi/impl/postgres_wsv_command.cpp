/**
 * Copyright Soramitsu Co., Ltd. 2017 All Rights Reserved.
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
#include <iostream>

namespace iroha {
  namespace ametsuchi {

    PostgresWsvCommand::PostgresWsvCommand(pqxx::nontransaction &transaction)
        : transaction_(transaction) {}

    bool PostgresWsvCommand::insertAccount(const model::Account &account) {
      std::stringstream permissions;
      permissions << account.permissions.add_signatory
                  << account.permissions.can_transfer
                  << account.permissions.create_accounts
                  << account.permissions.create_assets
                  << account.permissions.create_domains
                  << account.permissions.issue_assets
                  << account.permissions.read_all_accounts
                  << account.permissions.remove_signatory
                  << account.permissions.set_permissions
                  << account.permissions.set_quorum;
      try {
        transaction_.exec(
            "INSERT INTO account(\n"
            "            account_id, domain_id, quorum, status, "
            "transaction_count, \n"
            "            permissions)\n"
            "    VALUES (" +
            transaction_.quote(account.account_id) + ", " +
            transaction_.quote(account.domain_name) + ", " +
            transaction_.quote(account.quorum) + ", " +
            /*account.status*/ transaction_.quote(0) + ", " +
            /*account.transaction_count*/ transaction_.quote(0) +
            ", \n"
            "            " +
            transaction_.quote(permissions.str()) + ");");
      } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return false;
      }
      return true;
    }

    bool PostgresWsvCommand::insertAsset(const model::Asset &asset) {
      uint32_t precision = asset.precision;
      try {
        transaction_.exec(
            "INSERT INTO asset(\n"
            "            asset_id, domain_id, \"precision\", data)\n"
            "    VALUES (" +
            transaction_.quote(asset.asset_id) + ", " +
            transaction_.quote(asset.domain_id) + ", " +
            transaction_.quote(precision) + ", " + /*asset.data*/ "NULL" +
            ");");
      } catch (const std::exception &e) {
        return false;
      }
      return true;
    }

    bool PostgresWsvCommand::upsertAccountAsset(
        const model::AccountAsset &asset) {
      try {
        transaction_.exec(
            "INSERT INTO public.account_has_asset(\n"
            "            account_id, asset_id, amount, permissions)\n"
            "    VALUES (" +
            transaction_.quote(asset.account_id) + ", " +
            transaction_.quote(asset.asset_id) + ", " +
            transaction_.quote(asset.balance) + ", " +
            /*asset.permissions*/ transaction_.quote(0) +
            ")\n"
            "    ON CONFLICT (account_id, asset_id)\n"
            "    DO UPDATE SET \n"
            "        amount=EXCLUDED.amount, \n"
            "        permissions=EXCLUDED.permissions;");
      } catch (const std::exception &e) {
        return false;
      }
      return true;
    }

    bool PostgresWsvCommand::insertSignatory(
        const ed25519::pubkey_t &signatory) {
      try {
        pqxx::binarystring public_key(signatory.data(), signatory.size());
        transaction_.exec(
            "INSERT INTO signatory(\n"
            "            public_key)\n"
            "    VALUES (" +
            transaction_.quote(public_key) + ");");
      } catch (const std::exception &e) {
        return false;
      }
      return true;
    }

    bool PostgresWsvCommand::insertAccountSignatory(
        const std::string &account_id, const ed25519::pubkey_t &signatory) {
      pqxx::binarystring public_key(signatory.data(), signatory.size());
      try {
        transaction_.exec(
            "INSERT INTO account_has_signatory(\n"
            "            account_id, public_key)\n"
            "    VALUES (" +
            transaction_.quote(account_id) + ", " +
            transaction_.quote(public_key) + ");");
      } catch (const std::exception &e) {
        return false;
      }
      return true;
    }

    bool PostgresWsvCommand::deleteAccountSignatory(
        const std::string &account_id, const ed25519::pubkey_t &signatory) {
      pqxx::binarystring public_key(signatory.data(), signatory.size());
      try {
        transaction_.exec(
            "DELETE FROM account_has_signatory\n"
            " WHERE account_id=" +
            transaction_.quote(account_id) + " AND public_key=" +
            transaction_.quote(public_key) + ";");
      } catch (const std::exception &e) {
        return false;
      }
      return true;
    }

    bool PostgresWsvCommand::insertPeer(const model::Peer &peer) {
      pqxx::binarystring public_key(peer.pubkey.data(), peer.pubkey.size());
      try {
        transaction_.exec(
            "INSERT INTO peer(\n"
            "            public_key, address, state)\n"
            "    VALUES (" +
            transaction_.quote(public_key) + ", " +
            transaction_.quote(peer.address) + ", " +
            /*peer.state*/ transaction_.quote(0) + ");");
      } catch (const std::exception &e) {
        return false;
      }
      return true;
    }

    bool PostgresWsvCommand::deletePeer(const model::Peer &peer) {
      pqxx::binarystring public_key(peer.pubkey.data(), peer.pubkey.size());
      try {
        transaction_.exec(
            "DELETE FROM peer\n"
            " WHERE public_key=" +
            transaction_.quote(public_key) + " AND address=" +
            transaction_.quote(peer.address) + ";");
      } catch (const std::exception &e) {
        return false;
      }
      return true;
    }

    bool PostgresWsvCommand::insertDomain(const model::Domain &domain) {
      try {
        transaction_.exec(
            "INSERT INTO domain(\n"
            "            domain_id, open)\n"
            "    VALUES (" +
            transaction_.quote(domain.domain_id) + ", " +
            /*domain.open*/ transaction_.quote(true) + ");");
      } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return false;
      }
      return true;
    }

    bool PostgresWsvCommand::updateAccount(const model::Account &account) {
      std::stringstream permissions;
      permissions << account.permissions.add_signatory
                  << account.permissions.can_transfer
                  << account.permissions.create_accounts
                  << account.permissions.create_assets
                  << account.permissions.create_domains
                  << account.permissions.issue_assets
                  << account.permissions.read_all_accounts
                  << account.permissions.remove_signatory
                  << account.permissions.set_permissions
                  << account.permissions.set_quorum;
      try {
        transaction_.exec(
            "UPDATE account\n"
            "   SET quorum=" +
            transaction_.quote(account.quorum) + ", status=" +
            /*account.status*/ transaction_.quote(0) + ", transaction_count=" +
            /*account.transaction_count*/ transaction_.quote(0) +
            ", permissions=" + transaction_.quote(permissions.str()) +
            "\n"
            " WHERE account_id=" +
            transaction_.quote(account.account_id) + ";");
      } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return false;
      }
      return true;
    }
  }  // namespace ametsuchi
}  // namespace iroha
