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

#ifndef IROHA_AMOUNT_UTILS_HPP
#define IROHA_AMOUNT_UTILS_HPP

#include <boost/multiprecision/cpp_int.hpp>

#include "builders/default_builders.hpp"
#include "builders/protobuf/common_objects/proto_amount_builder.hpp"
#include "common/result.hpp"

namespace shared_model {
  namespace detail {
    boost::multiprecision::uint256_t increaseValuePrecision(
        boost::multiprecision::uint256_t value, int degree);

    /**
     * Sums up two amounts.
     * Result is returned
     * @param a left term
     * @param b right term
     */
    iroha::expected::PolymorphicResult<shared_model::interface::Amount,
                                       std::string>
    operator+(const shared_model::interface::Amount &a,
              const shared_model::interface::Amount &b);

    /**
     * Subtracts two amounts.
     * Result is returned
     * @param a left term
     * @param b right term
     */
    iroha::expected::PolymorphicResult<shared_model::interface::Amount,
                                       std::string>
    operator-(const shared_model::interface::Amount &a,
              const shared_model::interface::Amount &b);

    /**
     * Make amount with bigger precision
     * Result is returned
     * @param a amount
     * @param b right term
     */
    iroha::expected::PolymorphicResult<shared_model::interface::Amount,
                                       std::string>
    makeAmountWithPrecision(const shared_model::interface::Amount &amount,
                            const int new_precision);

    int compareAmount(const shared_model::interface::Amount &a,
                      const shared_model::interface::Amount &b);
  }  // namespace detail
}  // namespace shared_model
#endif  // IROHA_AMOUNT_UTILS_HPP
