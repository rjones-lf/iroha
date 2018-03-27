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

#include <pqxx/connection>

#include "postgres_wsv_common.hpp"

namespace iroha {
  namespace ametsuchi {
    class PostgresWsvQuery : public WsvQuery {
     public:
      explicit PostgresWsvQuery(pqxx::nontransaction &transaction);
      PostgresWsvQuery(std::unique_ptr<pqxx::lazyconnection> connection,
                       std::unique_ptr<pqxx::nontransaction> transaction);
      boost::optional<std::vector<shared_model::interface::types::RoleIdType>>
      getAccountRoles(const shared_model::interface::types::AccountIdType
                          &account_id) override;

      boost::optional<
          std::vector<shared_model::interface::types::PermissionNameType>>
      getRolePermissions(
          const shared_model::interface::types::RoleIdType &role_name) override;

      boost::optional<std::shared_ptr<shared_model::interface::Account>>
      getAccount(const shared_model::interface::types::AccountIdType
                     &account_id) override;
      boost::optional<std::string> getAccountDetail(
          const shared_model::interface::types::AccountIdType &account_id)
          override;
      boost::optional<std::vector<shared_model::interface::types::PubkeyType>>
      getSignatories(const shared_model::interface::types::AccountIdType
                         &account_id) override;
      boost::optional<std::shared_ptr<shared_model::interface::Asset>> getAsset(
          const shared_model::interface::types::AssetIdType &asset_id) override;
      boost::optional<std::shared_ptr<shared_model::interface::AccountAsset>>
      getAccountAsset(
          const shared_model::interface::types::AccountIdType &account_id,
          const shared_model::interface::types::AssetIdType &asset_id) override;
      boost::optional<
          std::vector<std::shared_ptr<shared_model::interface::Peer>>>
      getPeers() override;
      boost::optional<std::vector<shared_model::interface::types::RoleIdType>>
      getRoles() override;
      boost::optional<std::shared_ptr<shared_model::interface::Domain>>
      getDomain(const shared_model::interface::types::DomainIdType &domain_id)
          override;
      bool hasAccountGrantablePermission(
          const shared_model::interface::types::AccountIdType
              &permitee_account_id,
          const shared_model::interface::types::AccountIdType &account_id,
          const shared_model::interface::types::PermissionNameType
              &permission_id) override;

     private:
      std::unique_ptr<pqxx::lazyconnection> connection_ptr_;
      std::unique_ptr<pqxx::nontransaction> transaction_ptr_;

      pqxx::nontransaction &transaction_;
      logger::Logger log_;

      using ExecuteType = decltype(makeExecuteOptional(transaction_, log_));
      ExecuteType execute_;
    };
  }  // namespace ametsuchi
}  // namespace iroha

#endif  // IROHA_POSTGRES_WSV_QUERY_HPP
