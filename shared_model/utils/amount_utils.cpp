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

#include "utils/amount_utils.hpp"

#include <boost/format.hpp>

#include "builders/default_builders.hpp"
#include "builders/protobuf/common_objects/proto_amount_builder.hpp"

namespace shared_model {
  namespace detail {
    const boost::multiprecision::uint256_t ten = 10;

    boost::multiprecision::uint256_t increaseValuePrecision(
        boost::multiprecision::uint256_t value, int degree) {
      return value * pow(ten, degree);
    }

    /**
     * Sums up two amounts.
     * Result is returned
     * @param a left term
     * @param b right term
     */
    iroha::expected::PolymorphicResult<shared_model::interface::Amount,
                                       std::string>
    operator+(const shared_model::interface::Amount &a,
              const shared_model::interface::Amount &b) {
      auto max_precision = std::max(a.precision(), b.precision());
      auto val_a =
          increaseValuePrecision(a.intValue(), max_precision - a.precision());
      auto val_b =
          increaseValuePrecision(b.intValue(), max_precision - b.precision());
      if (val_a < a.intValue() || val_b < b.intValue() || val_a + val_b < val_a
          || val_a + val_b < val_b) {
        return iroha::expected::makeError(std::make_shared<std::string>(
            (boost::format("addition overflows (%s + %s)") % a.intValue().str()
             % b.intValue().str())
                .str()));
      }
      return shared_model::builder::AmountBuilderWithoutValidator()
          .precision(max_precision)
          .intValue(val_a + val_b)
          .build();
    }

    /**
     * Subtracts two amounts.
     * Result is returned
     * @param a left term
     * @param b right term
     */
    iroha::expected::PolymorphicResult<shared_model::interface::Amount,
                                       std::string>
    operator-(const shared_model::interface::Amount &a,
              const shared_model::interface::Amount &b) {
      // check if a greater than b
      if (a.intValue() < b.intValue()) {
        return iroha::expected::makeError(std::make_shared<std::string>(
            (boost::format("minuend is smaller than subtrahend (%s - %s)")
             % a.intValue().str() % b.intValue().str())
                .str()));
      }
      auto max_precision = std::max(a.precision(), b.precision());
      auto val_a =
          increaseValuePrecision(a.intValue(), max_precision - a.precision());
      auto val_b =
          increaseValuePrecision(b.intValue(), max_precision - b.precision());
      if (val_a < a.intValue() || val_b < b.intValue()) {
        return iroha::expected::makeError(
            std::make_shared<std::string>("new precision overflows number"));
      }
      return shared_model::builder::AmountBuilderWithoutValidator()
          .precision(max_precision)
          .intValue(val_a - val_b)
          .build();
    }

    /**
     * Make amount with bigger precision
     * Result is returned
     * @param a amount
     * @param b right term
     */
    iroha::expected::PolymorphicResult<shared_model::interface::Amount,
                                       std::string>
    makeAmountWithPrecision(const shared_model::interface::Amount &amount,
                            const int new_precision) {
      if (amount.precision() > new_precision) {
        return iroha::expected::makeError(std::make_shared<std::string>(
            (boost::format("new precision is smaller than current (%d < %d)")
             % new_precision % amount.precision())
                .str()));
      }
      auto val_amount = increaseValuePrecision(
          amount.intValue(), new_precision - amount.precision());
      if (val_amount < amount.intValue()) {
        return iroha::expected::makeError(
            std::make_shared<std::string>("operation overflows number"));
      }
      return shared_model::builder::AmountBuilderWithoutValidator()
          .precision(new_precision)
          .intValue(val_amount)
          .build();
    }

    int compareAmount(const shared_model::interface::Amount &a,
                      const shared_model::interface::Amount &b) {
      if (a.precision() == b.precision()) {
        return (a.intValue() < b.intValue())
            ? -1
            : (a.intValue() > b.intValue()) ? 1 : 0;
      }
      // when different precisions transform to have the same scale
      auto max_precision = std::max(a.precision(), b.precision());

      auto val1 =
          increaseValuePrecision(a.intValue(), max_precision - a.precision());
      auto val2 =
          increaseValuePrecision(b.intValue(), max_precision - b.precision());
      return (val1 < val2) ? -1 : (val1 > val2) ? 1 : 0;
    }
  }  // namespace detail
}  // namespace shared_model
