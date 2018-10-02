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

#include <soci/soci.h>

#include "interfaces/common_objects/common_objects_factory.hpp"
#include "logger/logger.hpp"

namespace iroha {
  namespace ametsuchi {
    class PostgresWsvQuery : public WsvQuery {
     public:
      PostgresWsvQuery(
          soci::session &sql,
          std::shared_ptr<shared_model::interface::CommonObjectsFactory>
              factory);

      PostgresWsvQuery(
          std::unique_ptr<soci::session> sql,
          std::shared_ptr<shared_model::interface::CommonObjectsFactory>
              factory);

      boost::optional<std::vector<shared_model::interface::types::RoleIdType>>
      getAccountRoles(const shared_model::interface::types::AccountIdType
                          &account_id) override;

      boost::optional<shared_model::interface::RolePermissionSet>
      getRolePermissions(
          const shared_model::interface::types::RoleIdType &role_name) override;

      boost::optional<std::shared_ptr<shared_model::interface::Account>>
      getAccount(const shared_model::interface::types::AccountIdType
                     &account_id) override;

      boost::optional<std::string> getAccountDetail(
          const shared_model::interface::types::AccountIdType &account_id,
          const shared_model::interface::types::AccountDetailKeyType &key = "",
          const shared_model::interface::types::AccountIdType &writer =
              "") override;

      boost::optional<std::vector<shared_model::interface::types::PubkeyType>>
      getSignatories(const shared_model::interface::types::AccountIdType
                         &account_id) override;

      boost::optional<std::shared_ptr<shared_model::interface::Asset>> getAsset(
          const shared_model::interface::types::AssetIdType &asset_id) override;

      boost::optional<
          std::vector<std::shared_ptr<shared_model::interface::AccountAsset>>>
      getAccountAssets(const shared_model::interface::types::AccountIdType
                           &account_id) override;

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
          shared_model::interface::permissions::Grantable permission) override;

     private:
      /**
       * Transforms result to optional
       * value -> optional<value>
       * error -> nullopt
       * @tparam T type of object inside
       * @param result BuilderResult
       * @return optional<T>
       */
      template <typename T>
      boost::optional<std::shared_ptr<T>> fromResult(
          shared_model::interface::CommonObjectsFactory::FactoryResult<
              std::unique_ptr<T>> &&result);

      /**
       * Executes given lambda of type F, catches exceptions if any, logs the
       * message, and returns an optional rowset<T>
       */
      template <typename T, typename F>
      auto execute(F &&f) -> boost::optional<soci::rowset<T>>;

      // TODO andrei 24.09.2018: IR-1718 Consistent soci::session fields in
      // storage classes
      std::unique_ptr<soci::session> psql_;
      soci::session &sql_;
      std::shared_ptr<shared_model::interface::CommonObjectsFactory> factory_;
      logger::Logger log_;
    };
  }  // namespace ametsuchi
}  // namespace iroha

#endif  // IROHA_POSTGRES_WSV_QUERY_HPP
