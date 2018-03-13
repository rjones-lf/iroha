/**
 * Copyright Soramitsu Co., Ltd. 2018 All Rights Reserved.
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

#ifndef IROHA_QUERY_ERROR_RESPONSE_VISITOR_HPP
#define IROHA_QUERY_ERROR_RESPONSE_VISITOR_HPP

#include <boost/variant.hpp>
#include "interfaces/query_responses/error_query_response.hpp"
#include "utils/polymorphic_wrapper.hpp"

namespace shared_model {
  namespace interface {
    template <typename Error>
    class QueryErrorResponseChecker : public boost::static_visitor<bool> {
     public:
      bool operator()(
          const shared_model::detail::PolymorphicWrapper<
              shared_model::interface::ErrorQueryResponse> &status) const {
        using ExpectedError = shared_model::detail::PolymorphicWrapper<Error>;
        try {
          auto _ = boost::get<ExpectedError>(status->get());
          return true;
        } catch (...) {
        }
        return false;
      }

      template <typename T>
      bool operator()(
          const shared_model::detail::PolymorphicWrapper<T> &) const {
        return false;
      }
    };
  }  // namespace interface
}  // namespace shared_model

#endif  // IROHA_QUERY_ERROR_RESPONSE_VISITOR_HPP
