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

#include "ametsuchi/impl/postgres_wsv_query.hpp"

#include "model/account.hpp"
#include "model/account_asset.hpp"
#include "model/asset.hpp"
#include "model/domain.hpp"
#include "model/peer.hpp"

namespace iroha {
  namespace ametsuchi {

    const std::string kRoleId = "role_id";
    const char *kAccountNotFound = "Account {} not found";
    const std::string kPublicKey = "public_key";
    const std::string kAssetId = "asset_id";
    const std::string kAccountId = "account_id";
    const std::string kDomainId = "domain_id";

    PostgresWsvQuery::PostgresWsvQuery(pqxx::nontransaction &transaction)
        : transaction_(transaction),
          log_(logger::log("PostgresWsvQuery")),
          execute_{makeExecute(transaction_, log_)} {}

    bool PostgresWsvQuery::hasAccountGrantablePermission(
        const std::string &permitee_account_id,
        const std::string &account_id,
        const std::string &permission_id) {
      return execute_(
                 "SELECT * FROM account_has_grantable_permissions WHERE "
                 "permittee_account_id = "
                 + transaction_.quote(permitee_account_id)
                 + " AND account_id = "
                 + transaction_.quote(account_id)
                 + " AND permission_id = "
                 + transaction_.quote(permission_id)
                 + ";")
          | [](const auto &result) { return result.size() == 1; };
    }

    nonstd::optional<std::vector<std::string>>
    PostgresWsvQuery::getAccountRoles(const std::string &account_id) {
      return execute_(
                 "SELECT role_id FROM account_has_roles WHERE account_id = "
                 + transaction_.quote(account_id)
                 + ";")
          | [&](const auto &result) {
              return transform<std::string>(result, [](const auto &row) {
                return row.at(kRoleId).c_str();
              });
            };
    }

    nonstd::optional<std::vector<std::string>>
    PostgresWsvQuery::getRolePermissions(const std::string &role_name) {
      return execute_(
                 "SELECT permission_id FROM role_has_permissions WHERE role_id "
                 "= "
                 + transaction_.quote(role_name)
                 + ";")
          | [&](const auto &result) {
              return transform<std::string>(result, [](const auto &row) {
                return row.at("permission_id").c_str();
              });
            };
    }

    nonstd::optional<std::vector<std::string>> PostgresWsvQuery::getRoles() {
      return execute_("SELECT role_id FROM role;") | [&](const auto &result) {
        return transform<std::string>(
            result, [](const auto &row) { return row.at(kRoleId).c_str(); });
      };
    }

    nonstd::optional<model::Account> PostgresWsvQuery::getAccount(
        const std::string &account_id) {
      return execute_("SELECT * FROM account WHERE account_id = "
                      + transaction_.quote(account_id)
                      + ";")
                 | [&](const auto &result) -> nonstd::optional<model::Account> {
        if (result.empty()) {
          log_->info(kAccountNotFound, account_id);
          return nonstd::nullopt;
        }
        model::Account account;
        auto row = result.at(0);
        row.at(kAccountId) >> account.account_id;
        row.at(kDomainId) >> account.domain_id;
        row.at("quorum") >> account.quorum;
        row.at("data") >> account.json_data;
        return account;
      };
    }

    nonstd::optional<std::string> PostgresWsvQuery::getAccountDetail(
        const std::string &account_id,
        const std::string &creator_account_id,
        const std::string &detail) {
      return execute_("SELECT data#>>"
                      + transaction_.quote(
                            "{" + creator_account_id + ", " + detail + "}")
                      + " FROM account WHERE account_id = "
                      + transaction_.quote(account_id)
                      + ";")
                 | [&](const auto &result) -> nonstd::optional<std::string> {
        if (result.empty()) {
          log_->info(kAccountNotFound, account_id);
          return nonstd::nullopt;
        }
        auto row = result.at(0);
        std::string res;
        row.at(0) >> res;

        // if res is empty, then that key does not exist for this account
        if (res.empty()) {
          return nonstd::nullopt;
        }
        return res;
      };
    }

    nonstd::optional<std::vector<pubkey_t>> PostgresWsvQuery::getSignatories(
        const std::string &account_id) {
      return execute_(
                 "SELECT public_key FROM account_has_signatory WHERE "
                 "account_id = "
                 + transaction_.quote(account_id)
                 + ";")
          |
          [&](const auto &result) {
            return transform<pubkey_t>(result, [&](const auto &row) {
              pqxx::binarystring public_key_str(row.at(kPublicKey));
              pubkey_t pubkey;
              std::copy(
                  public_key_str.begin(), public_key_str.end(), pubkey.begin());
              return pubkey;
            });
          };
    }

    nonstd::optional<model::Asset> PostgresWsvQuery::getAsset(
        const std::string &asset_id) {
      pqxx::result result;
      return execute_("SELECT * FROM asset WHERE asset_id = "
                      + transaction_.quote(asset_id)
                      + ";")
                 | [&](const auto &result) -> nonstd::optional<model::Asset> {
        if (result.empty()) {
          log_->info("Asset {} not found", asset_id);
          return nonstd::nullopt;
        }
        model::Asset asset;
        auto row = result.at(0);
        row.at(kAssetId) >> asset.asset_id;
        row.at(kDomainId) >> asset.domain_id;
        int32_t precision;
        row.at("precision") >> precision;
        asset.precision = precision;
        return asset;
      };
    }

    nonstd::optional<model::AccountAsset> PostgresWsvQuery::getAccountAsset(
        const std::string &account_id, const std::string &asset_id) {
      return execute_("SELECT * FROM account_has_asset WHERE account_id = "
                      + transaction_.quote(account_id)
                      + " AND asset_id = "
                      + transaction_.quote(asset_id)
                      + ";")
                 | [&](const auto &result)
                 -> nonstd::optional<model::AccountAsset> {
        if (result.empty()) {
          log_->info("Account {} does not have asset {}", account_id, asset_id);
          return nonstd::nullopt;
        }
        model::AccountAsset asset;
        auto row = result.at(0);
        row.at(kAccountId) >> asset.account_id;
        row.at(kAssetId) >> asset.asset_id;
        std::string amount_str;
        row.at("amount") >> amount_str;
        asset.balance = Amount::createFromString(amount_str).value();
        return asset;
      };
    }

    nonstd::optional<model::Domain> PostgresWsvQuery::getDomain(
        const std::string &domain_id) {
      return execute_("SELECT * FROM domain WHERE domain_id = "
                      + transaction_.quote(domain_id)
                      + ";")
                 | [&](const auto &result) -> nonstd::optional<model::Domain> {
        if (result.empty()) {
          log_->info("Domain {} not found", domain_id);
          return nonstd::nullopt;
        }
        model::Domain domain;
        auto row = result.at(0);
        row.at(kDomainId) >> domain.domain_id;
        row.at("default_role") >> domain.default_role;
        return domain;
      };
    }

    nonstd::optional<std::vector<model::Peer>> PostgresWsvQuery::getPeers() {
      pqxx::result result;
      return execute_("SELECT * FROM peer;") | [&](const auto &result) {
        return transform<model::Peer>(result, [](const auto &row) {
          model::Peer peer;
          pqxx::binarystring public_key_str(row.at(kPublicKey));
          pubkey_t pubkey;
          std::copy(
              public_key_str.begin(), public_key_str.end(), pubkey.begin());
          peer.pubkey = pubkey;
          row.at("address") >> peer.address;
          return peer;
        });
      };
    }
  }  // namespace ametsuchi
}  // namespace iroha
