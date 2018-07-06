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

#ifndef IROHA_RESULT_FIXTURE_HPP
#define IROHA_RESULT_FIXTURE_HPP

#include "common/result.hpp"

namespace framework {
  namespace expected {
    template <typename ResultType>
    using ValueOf = iroha::expected::ValueOf<ResultType>;
    template <typename ResultType>
    using ErrorOf = iroha::expected::ErrorOf<ResultType>;
    /**
     * @return optional with value if present
     *         otherwise none
     */
    template <typename ResultType>
    boost::optional<ValueOf<ResultType>> val(const ResultType &res) noexcept {
      using RetType = boost::optional<ValueOf<ResultType>>;
      return iroha::visit_in_place(
          res,
          [](ValueOf<ResultType> v) { return RetType(v); },
          [](ErrorOf<ResultType> e) -> RetType { return {}; });
    }

    /**
     * @return optional with error if present
     *         otherwise none
     */
    template <typename ResultType>
    boost::optional<ErrorOf<ResultType>> err(const ResultType &res) noexcept {
      using RetType = boost::optional<ErrorOf<ResultType>>;
      return iroha::visit_in_place(
          res,
          [](ValueOf<ResultType> v) -> RetType { return {}; },
          [](ErrorOf<ResultType> e) { return RetType(e); });
    }
  }  // namespace expected
}  // namespace framework

#endif  // IROHA_RESULT_FIXTURE_HPP
