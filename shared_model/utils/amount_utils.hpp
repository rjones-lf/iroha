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

#include "builders/protobuf/common_objects/proto_amount_builder.hpp"

/**
 * Sums up two amounts.
 * Requires to have the same scale.
 * Otherwise nullopt is returned
 * @param a left term
 * @param b right term
 * @param optional result
 */
boost::optional<std::shared_ptr<shared_model::interface::Amount>> operator+(
    const shared_model::interface::Amount &a,
    const shared_model::interface::Amount &b) {
  // check precisions
  if (a.precision() != b.precision()) {
    return boost::none;
  }
  auto res = shared_model::builder::AmountBuilderWithoutValidator()
                 .precision(a.precision())
                 .intValue(a.intValue() + b.intValue())
                 .build();
  return res.match(
      [&a, &b](const iroha::expected::Value<
               std::shared_ptr<shared_model::interface::Amount>> &result) {
        // check overflow
        if (result.value->intValue() < a.intValue()
            or result.value->intValue() < b.intValue()) {
          return boost::optional<
              std::shared_ptr<shared_model::interface::Amount>>(boost::none);
        }
        return boost::optional<
            std::shared_ptr<shared_model::interface::Amount>>(result.value);
      },
      [](const auto &err) {
        return boost::optional<
            std::shared_ptr<shared_model::interface::Amount>>(boost::none);
      });
}

/**
 * Subtracts two amounts.
 * Requires to have the same scale.
 * Otherwise nullopt is returned
 * @param a left term
 * @param b right term
 * @param optional result
 */
boost::optional<std::shared_ptr<shared_model::interface::Amount>> operator-(
    const shared_model::interface::Amount &a,
    const shared_model::interface::Amount &b) {
  // check precisions
  if (a.precision() != b.precision()) {
    return boost::none;
  }
  // check if a greater than b
  if (a.intValue() < b.intValue()) {
    return boost::none;
  }
  auto res = shared_model::builder::AmountBuilderWithoutValidator()
                 .precision(a.precision())
                 .intValue(a.intValue() - b.intValue())
                 .build();
  return res.match(
      [&a, &b](const iroha::expected::Value<
               std::shared_ptr<shared_model::interface::Amount>> &result) {
        return boost::optional<
            std::shared_ptr<shared_model::interface::Amount>>(result.value);
      },
      [&](const auto &err) {
        return boost::optional<
            std::shared_ptr<shared_model::interface::Amount>>(boost::none);
      });
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
  auto val1 = a.intValue() * (int)std::pow(10, max_precision - a.precision());
  auto val2 = b.intValue() * (int)std::pow(10, max_precision - b.precision());
  return (val1 < val2) ? -1 : (val1 > val2) ? 1 : 0;
}

#endif  // IROHA_AMOUNT_UTILS_HPP
