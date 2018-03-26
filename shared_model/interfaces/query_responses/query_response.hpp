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

#ifndef IROHA_SHARED_MODEL_QUERY_RESPONSE_HPP
#define IROHA_SHARED_MODEL_QUERY_RESPONSE_HPP

#include <boost/variant.hpp>
#include "interfaces/base/primitive.hpp"
#include "interfaces/query_responses/account_asset_response.hpp"
#include "interfaces/query_responses/account_detail_response.hpp"
#include "interfaces/query_responses/account_response.hpp"
#include "interfaces/query_responses/asset_response.hpp"
#include "interfaces/query_responses/error_query_response.hpp"
#include "interfaces/query_responses/role_permissions.hpp"
#include "interfaces/query_responses/roles_response.hpp"
#include "interfaces/query_responses/signatories_response.hpp"
#include "interfaces/query_responses/transactions_response.hpp"

#ifndef DISABLE_BACKWARD
#include "model/query_response.hpp"
#endif

namespace shared_model {
  namespace interface {
    /**
     * Class QueryResponse(qr) provides container with concrete query responses
     * available in the system.
     * General note: this class is container for QRs but not a base class.
     */
    class QueryResponse : public PRIMITIVE(QueryResponse) {
     private:
      /// Shortcut type for polymorphic wrapper
      template <typename... Value>
      using w = boost::variant<detail::PolymorphicWrapper<Value>...>;

     public:
      /// Type of container with all concrete query response
      using QueryResponseVariantType = w<AccountAssetResponse,
                                         AccountDetailResponse,
                                         AccountResponse,
                                         ErrorQueryResponse,
                                         SignatoriesResponse,
                                         TransactionsResponse,
                                         AssetResponse,
                                         RolesResponse,
                                         RolePermissionsResponse>;

      /// Type of all available query responses
      using QueryResponseListType = QueryResponseVariantType::types;

      /**
       * @return reference to const variant with concrete qr
       */
      virtual const QueryResponseVariantType &get() const = 0;

      /**
       * @return hash of corresponding query
       */
      virtual const interface::types::HashType &queryHash() const = 0;

      // ------------------------| Primitive override |-------------------------

      std::string toString() const override {
        return boost::apply_visitor(detail::ToStringVisitor(), get());
      }

#ifndef DISABLE_BACKWARD
      OldModelType *makeOldModel() const override {
        auto query_response = boost::apply_visitor(
            detail::OldModelCreatorVisitor<OldModelType *>(), get());
        using hashType = decltype(query_response->query_hash);
        query_response->query_hash = queryHash().makeOldModel<hashType>();
        return query_response;
      }
#endif

      bool operator==(const ModelType &rhs) const override {
        return queryHash() == rhs.queryHash() and get() == rhs.get();
      }
    };
  }  // namespace interface
}  // namespace shared_model
#endif  // IROHA_SHARED_MODEL_QUERY_RESPONSE_HPP
