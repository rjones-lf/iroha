/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "utils/amount_utils.hpp"
#include <boost/format.hpp>

namespace shared_model {
  namespace detail {
    const boost::multiprecision::uint256_t ten = 10;

    boost::multiprecision::uint256_t increaseValuePrecision(
        const boost::multiprecision::uint256_t &value, int degree) {
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
      std::string val = (val_a + val_b).str();
      if (max_precision != 0) {
        val.insert((val.rbegin() + max_precision).base(), '.');
      }
      return iroha::expected::makeValue(
          std::make_shared<shared_model::interface::Amount>(std::move(val)));
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
      auto diff = val_a - val_b;
      std::string val = diff.str();
      if (max_precision != 0 && val.size() > max_precision + 1) {
        auto ptr = val.rbegin() + max_precision;
        val.insert(ptr.base(), '.');
      }
      return iroha::expected::makeValue(
          std::make_shared<shared_model::interface::Amount>(std::move(val)));
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
      std::string val = val_amount.str();
      if (new_precision != 0) {
        val.insert((val.rbegin() + new_precision).base(), '.');
      }
      return iroha::expected::makeValue(
          std::make_shared<shared_model::interface::Amount>(std::move(val)));
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
