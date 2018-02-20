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

#include "postgres_wsv_common.hpp"

namespace iroha {
  namespace ametsuchi {
    class PostgresWsvQuery : public WsvQuery {
     public:
      explicit PostgresWsvQuery(pqxx::nontransaction &transaction);
      nonstd::optional<std::vector<RoleIdType>> getAccountRoles(
          const AccountIdType &account_id) override;

      nonstd::optional<std::vector<PermissionNameType>> getRolePermissions(
          const RoleIdType &role_name) override;

      nonstd::optional<std::shared_ptr<shared_model::interface::Account>>
      getAccount(const AccountIdType &account_id) override;
      nonstd::optional<DetailType> getAccountDetail(
          const AccountIdType &account_id,
          const AccountIdType &creator_account_id,
          const DetailType &detail) override;
      nonstd::optional<std::vector<PubkeyType>> getSignatories(
          const AccountIdType &account_id) override;
      nonstd::optional<std::shared_ptr<shared_model::interface::Asset>>
      getAsset(const AssetIdType &asset_id) override;
      nonstd::optional<std::shared_ptr<shared_model::interface::AccountAsset>>
      getAccountAsset(const AccountIdType &account_id,
                      const AssetIdType &asset_id) override;
      nonstd::optional<
          std::vector<std::shared_ptr<shared_model::interface::Peer>>>
      getPeers() override;
      nonstd::optional<std::vector<RoleIdType>> getRoles() override;
      nonstd::optional<std::shared_ptr<shared_model::interface::Domain>>
      getDomain(const DomainIdType &domain_id) override;
      bool hasAccountGrantablePermission(
          const AccountIdType &permitee_account_id,
          const AccountIdType &account_id,
          const PermissionNameType &permission_id) override;

     private:
      pqxx::nontransaction &transaction_;
      logger::Logger log_;

      using ExecuteType = decltype(makeExecuteOptional(transaction_, log_));
      ExecuteType execute_;
    };
  }  // namespace ametsuchi
}  // namespace iroha

#endif  // IROHA_POSTGRES_WSV_QUERY_HPP
