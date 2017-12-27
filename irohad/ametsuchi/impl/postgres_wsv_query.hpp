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

#ifndef IROHA_POSTGRES_WSV_QUERY_HPP
#define IROHA_POSTGRES_WSV_QUERY_HPP

#include "ametsuchi/wsv_query.hpp"

#include <pqxx/nontransaction>

#include "logger/logger.hpp"

namespace iroha {
  namespace ametsuchi {
    class PostgresWsvQuery : public WsvQuery {
     public:
      explicit PostgresWsvQuery(pqxx::nontransaction &transaction);
      nonstd::optional<std::vector<std::string>> getAccountRoles(
          const std::string &account_id) override;

      nonstd::optional<std::vector<std::string>> getRolePermissions(
          const std::string &role_name) override;

      nonstd::optional<model::Account> getAccount(
          const std::string &account_id) override;
      nonstd::optional<std::string> getAccountDetail(
          const std::string &account_id,
          const std::string &creator_account_id,
          const std::string &detail) override;
      nonstd::optional<std::vector<pubkey_t>> getSignatories(
          const std::string &account_id) override;
      nonstd::optional<model::Asset> getAsset(
          const std::string &asset_id) override;
      nonstd::optional<model::AccountAsset> getAccountAsset(
          const std::string &account_id, const std::string &asset_id) override;
      nonstd::optional<std::vector<model::Peer>> getPeers() override;
      nonstd::optional<std::vector<std::string>> getRoles() override;
      nonstd::optional<model::Domain> getDomain(
          const std::string &domain_id) override;
      bool hasAccountGrantablePermission(
          const std::string &permitee_account_id,
          const std::string &account_id,
          const std::string &permission_id) override;

     private:
      pqxx::nontransaction &transaction_;

      logger::Logger log_;
    };
  }  // namespace ametsuchi
}  // namespace iroha

#endif  // IROHA_POSTGRES_WSV_QUERY_HPP
