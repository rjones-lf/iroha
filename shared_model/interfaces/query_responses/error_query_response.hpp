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

#ifndef IROHA_SHARED_MODEL_QUERY_ERROR_RESPONSE_HPP
#define IROHA_SHARED_MODEL_QUERY_ERROR_RESPONSE_HPP

#include <boost/variant.hpp>
#include "interfaces/base/primitive.hpp"
#include "interfaces/query_responses/error_responses/no_account_assets_error_response.hpp"
#include "interfaces/query_responses/error_responses/no_account_error_response.hpp"
#include "interfaces/query_responses/error_responses/no_asset_error_response.hpp"
#include "interfaces/query_responses/error_responses/no_roles_error_response.hpp"
#include "interfaces/query_responses/error_responses/no_signatories_error_response.hpp"
#include "interfaces/query_responses/error_responses/not_supported_error_response.hpp"
#include "interfaces/query_responses/error_responses/stateful_failed_error_response.hpp"
#include "interfaces/query_responses/error_responses/stateless_failed_error_response.hpp"

namespace shared_model {
  namespace interface {

    /**
     * QueryErrorResponse interface container for all concrete error responses
     * possible achieved in the system.
     */
    class ErrorQueryResponse
        : public Primitive<ErrorQueryResponse, iroha::model::ErrorResponse> {
     private:
      /// Shortcut type for polymorphic wrapper
      template <typename... Value>
      using w = boost::variant<detail::PolymorphicWrapper<Value>...>;

     public:
      /// Type of container with all concrete error query responses
      using QueryErrorResponseVariantType = w<StatelessFailedErrorResponse,
                                              StatefulFailedErrorResponse,
                                              NoAccountErrorResponse,
                                              NoAccountAssetsErrorResponse,
                                              NoSignatoriesErrorResponse,
                                              NotSupportedErrorResponse,
                                              NoAssetErrorResponse,
                                              NoRolesErrorResponse>;

      /// Type list with all concrete query error responses
      using QueryResponseListType = QueryErrorResponseVariantType::types;

      /**
       * @return reference to const variant with concrete error response
       */
      virtual const QueryErrorResponseVariantType &get() const = 0;

      // ------------------------| Primitive override |-------------------------

      std::string toString() const override {
        return boost::apply_visitor(detail::ToStringVisitor(), get());
      }

      OldModelType *makeOldModel() const override {
        return boost::apply_visitor(
            detail::OldModelCreatorVisitor<OldModelType *>(), get());
      }

      bool operator==(const ModelType &rhs) const override {
        return get() == rhs.get();
      }
    };
  }  // namespace interface
}  // namespace shared_model
#endif  // IROHA_SHARED_MODEL_QUERY_ERROR_RESPONSE_HPP
