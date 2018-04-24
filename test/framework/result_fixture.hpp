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
    /**
     * @return optional with value if present
     *         otherwise none
     */
    template <typename ResultType>
    boost::optional<typename ResultType::ValueType> val(const ResultType &res) {
      using VType = typename ResultType::ValueType;
      using EType = typename ResultType::ErrorType;
      return iroha::visit_in_place(
          res,
          [](VType v) { return boost::optional<VType>(v); },
          [](EType e) -> boost::optional<VType> { return {}; });
    }

    /**
     * @return optional with error if present
     *         otherwise none
     */
    template <typename ResultType>
    boost::optional<typename ResultType::ErrorType> err(const ResultType &res) {
      using VType = typename ResultType::ValueType;
      using EType = typename ResultType::ErrorType;
      return iroha::visit_in_place(
          res,
          [](VType v) -> boost::optional<EType> { return {}; },
          [](EType e) { return boost::optional<EType>(e); });
    }
  }  // namespace expected
}  // namespace framework

#endif  // IROHA_RESULT_FIXTURE_HPP
