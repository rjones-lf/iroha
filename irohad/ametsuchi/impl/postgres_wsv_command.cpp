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

#include <exception>
#include <iterator>
#include <numeric>

namespace iroha {
  namespace ametsuchi {
    PostgresWsvCommand::PostgresWsvCommand(pqxx::nontransaction &transaction)
        : transaction_(transaction), log_(logger::log("PostgresWsvCommand")) {}

    const char *kUnsuccesfulAlloc = "Unable to alloc: {}";

    template <typename T>
    std::string try_quote(PostgresWsvCommand *that,
                          std::initializer_list<T> l) noexcept {
      try {
        return try_quote(
            that,
            std::accumulate(l.begin(),
                            l.end(),
                            std::string(),
                            [](std::string s, auto next) { return s + next; }));
      } catch (std::bad_alloc &e) {
        that->log_->error(kUnsuccesfulAlloc, e.what());
        std::terminate();
      }
    }

    template <typename T>
    std::string try_quote(PostgresWsvCommand *that, const T &s) noexcept {
      try {
        return that->transaction_.quote(s);
      } catch (std::bad_alloc &e) {
        that->log_->error(kUnsuccesfulAlloc, e.what());
        std::terminate();
      }
    }

    std::string try_concat(PostgresWsvCommand *that,
                           std::initializer_list<const char *> l) noexcept {
      try {
        return std::accumulate(
            l.begin(), l.end(), std::string(), [](std::string s, auto next) {
              return s + next;
            });
      } catch (std::bad_alloc &e) {
        that->log_->error(kUnsuccesfulAlloc, e.what());
        std::terminate();
      }
    }

    bool try_exec(PostgresWsvCommand *that, const std::string &s) noexcept {
      try {
        that->transaction_.exec(s);
      } catch (const std::exception &e) {
        that->log_->error(e.what());
        return false;
      }
      return true;
    }

    bool PostgresWsvCommand::insertRole(const std::string &role_name) {
      auto rname = try_quote(this, role_name);
      auto list = {"INSERT INTO role(role_id) VALUES (", rname.c_str(), ");"};
      auto query = try_concat(this, list);
      return try_exec(this, query);
    }

    bool PostgresWsvCommand::insertAccountRole(const std::string &account_id,
                                               const std::string &role_name) {
      auto _account_id = try_quote(this, account_id);
      auto _role_name = try_quote(this, role_name);
      return try_exec(
          this,
          try_concat(
              this,
              {"INSERT INTO account_has_roles(account_id, role_id) VALUES (",
               _account_id.c_str(),
               ", ",
               _role_name.c_str(),
               ");"}));
    }

    bool PostgresWsvCommand::deleteAccountRole(const std::string &account_id,
                                               const std::string &role_name) {
      auto _account_id = try_quote(this, account_id);
      auto _role_name = try_quote(this, role_name);
      return try_exec(
          this,
          try_concat(this,
                     {"DELETE FROM account_has_roles WHERE account_id=",
                      _account_id.c_str(),
                      "AND role_id=",
                      _role_name.c_str(),
                      ";"}));
    }

    bool PostgresWsvCommand::insertRolePermissions(
        const std::string &role_id, const std::set<std::string> &permissions) {
      auto _role_id = try_quote(this, role_id);
      auto entry = [this, &_role_id](auto permission) {
        auto _permission = try_quote(this, permission);
        return try_concat(
            this, {"(", _role_id.c_str(), ", ", _permission.c_str(), ")"});
      };
      auto first = entry(*permissions.begin());
      auto _vals = std::accumulate(
          std::next(permissions.begin()),
          permissions.end(),
          first,
          [this, &entry](auto acc, auto x) {
            auto e = entry(x);
            return try_concat(this, {acc.c_str(), ", ", e.c_str()});
          });
      return try_exec(this,
                      try_concat(this,
                                 {"INSERT INTO role_has_permissions(role_id, "
                                  "permission_id) VALUES ",
                                  _vals.c_str(),
                                  ";"}));
    }

    bool PostgresWsvCommand::insertAccountGrantablePermission(
        const std::string &permittee_account_id,
        const std::string &account_id,
        const std::string &permission_id) {
      auto _permittee_account_id = try_quote(this, permittee_account_id);
      auto _account_id = try_quote(this, account_id);
      auto _permission_id = try_quote(this, permission_id);
      return try_exec(
          this,
          try_concat(this,
                     {"INSERT INTO "
                      "account_has_grantable_permissions(permittee_account_id, "
                      "account_id, permission_id) VALUES (",
                      _permittee_account_id.c_str(),
                      ", ",
                      _account_id.c_str(),
                      ", ",
                      _permission_id.c_str(),
                      ");"}));
    }

    bool PostgresWsvCommand::deleteAccountGrantablePermission(
        const std::string &permittee_account_id,
        const std::string &account_id,
        const std::string &permission_id) {
      auto _permittee_account_id = try_quote(this, permittee_account_id);
      auto _account_id = try_quote(this, account_id);
      auto _permission_id = try_quote(this, permission_id);
      return try_exec(
          this,
          try_concat(
              this,
              {"DELETE FROM public.account_has_grantable_permissions WHERE "
               "permittee_account_id=",
               _permittee_account_id.c_str(),
               " AND account_id=",
               _account_id.c_str(),
               " AND permission_id=",
               _permission_id.c_str(),
               " ;"}));
    }

    bool PostgresWsvCommand::insertAccount(const model::Account &account) {
      auto _account_id = try_quote(this, account.account_id);
      auto _domain_id = try_quote(this, account.domain_id);
      auto _quorum = try_quote(this, account.quorum);
      auto _default_tx_counter = try_quote(this, default_tx_counter);
      auto _json_data = try_quote(this, account.json_data);
      return try_exec(
          this,
          try_concat(this,
                     {"INSERT INTO account(account_id, domain_id, quorum, "
                      "transaction_count, data) VALUES (",
                      _account_id.c_str(),
                      ", ",
                      _domain_id.c_str(),
                      ", ",
                      _quorum.c_str(),
                      ", "
                      // Transaction counter
                      ,
                      _default_tx_counter.c_str(),
                      ", ",
                      _json_data.c_str(),
                      ");"}));
    }

    bool PostgresWsvCommand::insertAsset(const model::Asset &asset) {
      uint32_t precision = asset.precision;
      auto _asset_id = try_quote(this, asset.asset_id);
      auto _domain_id = try_quote(this, asset.domain_id);
      auto _precision = try_quote(this, precision);
      return try_exec(
          this,
          try_concat(
              this,
              {"INSERT INTO asset(asset_id, domain_id, \"precision\", data) "
               "VALUES (",
               _asset_id.c_str(),
               ", ",
               _domain_id.c_str(),
               ", ",
               _precision.c_str(),
               ", ",
               /*asset.data*/ "NULL",
               ");"}));
    }

    bool PostgresWsvCommand::upsertAccountAsset(
        const model::AccountAsset &asset) {
      auto _account_id = try_quote(this, asset.account_id);
      auto _asset_id = try_quote(this, asset.asset_id);
      auto _balance = try_quote(this, asset.balance.to_string());
      return try_exec(
          this,
          try_concat(
              this,
              {"INSERT INTO account_has_asset(account_id, asset_id, amount) "
               "VALUES (",
               _account_id.c_str(),
               ", ",
               _asset_id.c_str(),
               ", ",
               _balance.c_str(),
               ") ON CONFLICT (account_id, asset_id) DO UPDATE SET "
               "amount = EXCLUDED.amount;"}));
    }

    bool PostgresWsvCommand::insertSignatory(const pubkey_t &signatory) {
      pqxx::binarystring public_key(signatory.data(), signatory.size());
      auto _public_key = try_quote(this, public_key);
      return try_exec(this,
                      try_concat(this,
                                 {"INSERT INTO signatory(public_key) VALUES (",
                                  _public_key.c_str(),
                                  ") ON CONFLICT DO NOTHING;"}));
    }

    bool PostgresWsvCommand::insertAccountSignatory(
        const std::string &account_id, const pubkey_t &signatory) {
      pqxx::binarystring public_key(signatory.data(), signatory.size());
      auto _account_id = try_quote(this, account_id);
      auto _public_key = try_quote(this, public_key);
      return try_exec(
          this,
          try_concat(this,
                     {"INSERT INTO account_has_signatory(account_id, "
                      "public_key) VALUES (",
                      _account_id.c_str(),
                      ", ",
                      _public_key.c_str(),
                      ");"}));
    }

    bool PostgresWsvCommand::deleteAccountSignatory(
        const std::string &account_id, const pubkey_t &signatory) {
      pqxx::binarystring public_key(signatory.data(), signatory.size());
      auto _account_id = try_quote(this, account_id);
      auto _public_key = try_quote(this, public_key);
      return try_exec(
          this,
          try_concat(this,
                     {"DELETE FROM account_has_signatory WHERE account_id = ",
                      _account_id.c_str(),
                      " AND public_key = ",
                      _public_key.c_str(),
                      ";"}));
    }

    bool PostgresWsvCommand::deleteSignatory(const pubkey_t &signatory) {
      pqxx::binarystring public_key(signatory.data(), signatory.size());
      auto _public_key = try_quote(this, public_key);
      return try_exec(
          this,
          try_concat(
              this,
              {"DELETE FROM signatory WHERE public_key = ",
               _public_key.c_str(),
               " AND NOT EXISTS (SELECT 1 FROM account_has_signatory "
               "WHERE public_key = ",
               _public_key.c_str(),
               ") AND NOT EXISTS (SELECT 1 FROM peer WHERE public_key = ",
               _public_key.c_str(),
               ");"}));
    }

    bool PostgresWsvCommand::insertPeer(const model::Peer &peer) {
      pqxx::binarystring public_key(peer.pubkey.data(), peer.pubkey.size());
      auto _public_key = try_quote(this, public_key);
      auto _address = try_quote(this, peer.address);
      return try_exec(
          this,
          try_concat(this,
                     {"INSERT INTO peer(public_key, address) VALUES (",
                      _public_key.c_str(),
                      ", ",
                      _address.c_str(),
                      ");"}));
    }

    bool PostgresWsvCommand::deletePeer(const model::Peer &peer) {
      pqxx::binarystring public_key(peer.pubkey.data(), peer.pubkey.size());
      auto _public_key = try_quote(this, public_key);
      auto _address = try_quote(this, peer.address);
      return try_exec(this,
                      try_concat(this,
                                 {"DELETE FROM peer WHERE public_key = ",
                                  _public_key.c_str(),
                                  " AND address = ",
                                  _address.c_str(),
                                  ";"}));
    }

    bool PostgresWsvCommand::insertDomain(const model::Domain &domain) {
      auto _domain_id = try_quote(this, domain.domain_id);
      auto _default_role = try_quote(this, domain.default_role);
      return try_exec(
          this,
          try_concat(this,
                     {"INSERT INTO domain(domain_id, default_role) VALUES (",
                      _domain_id.c_str(),
                      ", ",
                      _default_role.c_str(),
                      ");"}));
    }

    bool PostgresWsvCommand::updateAccount(const model::Account &account) {
      auto _quorum = try_quote(this, account.quorum);
      auto _default_tx_counter = try_quote(this, default_tx_counter);
      auto _account_id = try_quote(this, account.account_id);
      return try_exec(this,
                      try_concat(this,
                                 {"UPDATE account\n"
                                  "   SET quorum=",
                                  _quorum.c_str(),
                                  ", transaction_count=",
                                  /*account.transaction_count*/
                                  _default_tx_counter.c_str(),
                                  "\n"
                                  " WHERE account_id=",
                                  _account_id.c_str(),
                                  ";"}));
    }

    bool PostgresWsvCommand::setAccountKV(const std::string &account_id,
                                          const std::string &creator_account_id,
                                          const std::string &key,
                                          const std::string &val) {
      auto _creator_account_id = try_quote(this, creator_account_id);
      auto _creator_account_id_json =
          try_quote(this, {"{", creator_account_id.c_str(), "}"});
      auto _empty_json = try_quote(this, "{}");
      auto _creator_and_key = try_quote(
          this, {"{", creator_account_id.c_str(), ", ", key.c_str(), "}"});
      auto _val = try_quote(this, {"\"", val.c_str(), "\""});
      auto _account_id = try_quote(this, account_id);
      return try_exec(
          this,
          try_concat(this,
                     {"UPDATE account SET data = jsonb_set(CASE WHEN data ?",
                      _creator_account_id.c_str(),
                      " THEN data ELSE jsonb_set(data, ",
                      _creator_account_id_json.c_str(),
                      ",",
                      _empty_json.c_str(),
                      ") END,",
                      _creator_and_key.c_str(),
                      ",",
                      _val.c_str(),
                      ") WHERE account_id=",
                      _account_id.c_str(),
                      ";"}));
    }

  }  // namespace ametsuchi
}  // namespace iroha
