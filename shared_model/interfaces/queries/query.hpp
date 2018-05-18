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

#ifndef IROHA_SHARED_MODEL_QUERY_HPP
#define IROHA_SHARED_MODEL_QUERY_HPP

#include <boost/variant.hpp>

#include "interfaces/base/signable.hpp"
#include "interfaces/common_objects/types.hpp"
#include "interfaces/queries/get_account.hpp"
#include "interfaces/queries/get_account_asset_transactions.hpp"
#include "interfaces/queries/get_account_assets.hpp"
#include "interfaces/queries/get_account_detail.hpp"
#include "interfaces/queries/get_account_transactions.hpp"
#include "interfaces/queries/get_asset_info.hpp"
#include "interfaces/queries/get_role_permissions.hpp"
#include "interfaces/queries/get_roles.hpp"
#include "interfaces/queries/get_signatories.hpp"
#include "interfaces/queries/get_transactions.hpp"
#include "utils/polymorphic_wrapper.hpp"
#include "utils/string_builder.hpp"
#include "utils/visitor_apply_for_all.hpp"

namespace shared_model {
  namespace interface {

    /**
     * Class Query provides container with one of concrete query available in
     * system.
     * General note: this class is container for queries but not a base class.
     */
    class Query : public Signable<Query> {
     private:
      /// Shortcut type for polymorphic wrapper
      template <typename... Value>
      using wrap = boost::variant<detail::PolymorphicWrapper<Value>...>;

     public:
      /// Type of variant, that handle concrete query
      using QueryVariantType = wrap<GetAccount,
                                    GetSignatories,
                                    GetAccountTransactions,
                                    GetAccountAssetTransactions,
                                    GetTransactions,
                                    GetAccountAssets,
                                    GetAccountDetail,
                                    GetRoles,
                                    GetRolePermissions,
                                    GetAssetInfo>;

      /// Types of concrete commands, in attached variant
      using QueryListType = QueryVariantType::types;

      /**
       * @return reference to const variant with concrete command
       */
      virtual const QueryVariantType &get() const = 0;

      /**
       * @return id of query creator
       */
      virtual const types::AccountIdType &creatorAccountId() const = 0;

      /**
       * Query counter - incremental variable reflect for number of sent to
       * system queries plus 1. Required for preventing replay attacks.
       * @return attached query counter
       */
      virtual types::CounterType queryCounter() const = 0;

      // ------------------------| Primitive override |-------------------------

      std::string toString() const override {
        return detail::PrettyStringBuilder()
            .init("Query")
            .append("creatorId", creatorAccountId())
            .append("queryCounter", std::to_string(queryCounter()))
            .append(Signable::toString())
            .append(boost::apply_visitor(detail::ToStringVisitor(), get()))
            .finalize();
      }

      bool operator==(const ModelType &rhs) const override {
        return this->get() == rhs.get();
      }
    };
  }  // namespace interface
}  // namespace shared_model
#endif  // IROHA_SHARED_MODEL_QUERY_HPP
