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

#include <utility>

#include "amount/amount.hpp"

namespace iroha {

  // to raise to power integer values
  int ipow(int base, int exp) {
    int result = 1;
    while (exp != 0) {
      if (exp & 1) result *= base;
      exp >>= 1;
      base *= base;
    }

    return result;
  }

  Amount::Amount() : value_(0), precision_(0) {}

  Amount::Amount(uint256_t value) : value_(value), precision_(0) {}

  Amount::Amount(uint256_t amount, uint8_t precision)
      : value_(amount), precision_(precision) {}

  Amount::Amount(const Amount &am)
      : value_(am.value_), precision_(am.precision_) {}

  Amount &Amount::operator=(const Amount &other) {
    // check for self-assignment
    if (&other == this) return *this;
    value_ = other.value_;
    precision_ = other.precision_;
    return *this;
  }

  Amount::Amount(Amount &&am) : value_(am.value_), precision_(am.precision_) {}

  Amount &Amount::operator=(Amount &&other) {
    std::swap(value_, other.value_);
    std::swap(precision_, other.precision_);
    return *this;
  }

  Amount Amount::percentage(uint256_t percents) const {
    uint256_t new_val = value_ * percents / 100;
    return {new_val, precision_};
  }

  Amount Amount::percentage(const Amount &am) const {
    // multiply two amount values
    uint256_t new_value = value_ * am.value_;

    // new value should be decreased by the scale of am to move floating point
    // to the left, as it is done when we multiply manually
    new_value /= uint256_t(pow(10, am.precision_));
    // to take percentage value we need divide by 100
    new_value /= 100;
    return {new_value, precision_};
  }

  Amount Amount::add(const Amount &other) const {
    auto new_val = value_ + other.value_;
    return {new_val, precision_};
  }

  Amount &Amount::operator+=(const Amount &other) {
    if (precision_ != other.precision_) {
      throw std::invalid_argument("precisions are not the same");
    }
    value_ += other.value_;
    return *this;
  }

  Amount Amount::subtract(const Amount &other) const {
    auto new_val = value_ - other.value_;
    return {new_val, precision_};
  }

  Amount &Amount::operator-=(const Amount &other) {
    if (precision_ != other.precision_) {
      throw std::invalid_argument("precisions are not the same");
    }
    value_ -= other.value_;
    return *this;
  }

  int Amount::compareTo(const Amount &other) const {
    if (precision_ == other.precision_) {
      return (value_ < other.value_) ? -1 : (value_ > other.value_) ? 1 : 0;
    }
    // when different precisions transform to have the same scale
    auto max_precision = std::max(precision_, other.precision_);
    auto val1 = value_ * ipow(10, max_precision - precision_);
    auto val2 = other.value_ * ipow(10, max_precision - other.precision_);
    return (val1 < val2) ? -1 : (val1 > val2) ? 1 : 0;
  }

  bool Amount::operator==(const Amount &other) const {
    return compareTo(other) == 0;
  }

  bool Amount::operator!=(const Amount &other) const {
    return compareTo(other) != 0;
  }

  bool Amount::operator<(const Amount &other) const {
    return compareTo(other) < 0;
  }

  bool Amount::operator>(const Amount &other) const {
    return compareTo(other) > 0;
  }

  bool Amount::operator<=(const Amount &other) const {
    return compareTo(other) <= 0;
  }

  bool Amount::operator>=(const Amount &other) const {
    return compareTo(other) >= 0;
  }

  std::string Amount::to_string() const {
    cpp_dec_float_50 float50(value_);
    float50 /= pow(10, precision_);
    return float50.str(precision_, std::ios_base::fixed);
  }
}
