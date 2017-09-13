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

namespace amount {

  using namespace boost::multiprecision;

  Amount::Amount() : value_(0), precision_(0) {}

  Amount::Amount(boost::multiprecision::uint256_t value)
      : value_(value), precision_(0) {}

  Amount::Amount(boost::multiprecision::uint256_t amount, uint8_t precision)
      : value_(amount), precision_(precision) {}

  Amount::Amount(const Amount &am)
      : value_(am.value_), precision_(am.precision_) {}

  Amount &Amount::operator=(const Amount &other) {
    // check for self-assignment
    if (&other == this) return *this;
    std::copy(&other.value_, &other.value_ + sizeof(decltype(value_)), &value_);
    std::copy(&other.precision_, &other.precision_ + sizeof(precision_),
              &precision_);
    return *this;
  }

  Amount::Amount(Amount &&am)
      : value_(std::move(am.value_)), precision_(std::move(am.precision_)) {}

  Amount &Amount::operator=(Amount &&other) {
    std::swap(value_, other.value_);
    std::swap(precision_, other.precision_);
    return *this;
  }

  Amount Amount::percentage(uint64_t percents) const {
    boost::multiprecision::uint256_t new_val = value_ * percents / 100;
    return {new_val, precision_};
  }

  Amount Amount::percentage(const Amount &am) const {
    boost::multiprecision::uint256_t new_value = value_ * am.value_;
    new_value /= uint256_t(std::pow(10, precision_ + am.precision_));
    return {new_value, precision_};
  }

  Amount Amount::operator+(const Amount &other) const {
    auto new_val = value_ + other.value_;
    return {new_val, precision_};
  }

  Amount &Amount::operator+=(const Amount &other) {
    value_ += other.value_;
    return *this;
  }

  Amount Amount::operator-(const Amount &other) const {
    auto new_val = value_ - other.value_;
    return {new_val, precision_};
  }

  Amount &Amount::operator-=(const Amount &other) {
    value_ -= other.value_;
    return *this;
  }

  bool Amount::operator==(const Amount &other) const {
    if (precision_ == other.precision_){
      return value_ == other.value_;
    }
    auto max_precision = std::max(precision_, other.precision_);
    auto val1 = value_ * uint256_t(std::pow(10, max_precision-precision_));
    auto val2 = other.value_ * uint256_t(std::pow(10, max_precision-other.precision_));
    return val1 == val2;
  }

  bool Amount::operator!=(const Amount &other) const {
    return !this->operator==(other);
  }

  bool Amount::operator<(const Amount &other) const {
    return value_ < other.value_;
  }

  bool Amount::operator>(const Amount &other) const {
    return value_ > other.value_;
  }

  bool Amount::operator<=(const Amount &other) const {
    return value_ <= other.value_;
  }

  bool Amount::operator>=(const Amount &other) const {
    return value_ >= other.value_;
  }

  std::string Amount::to_string() const {
    cpp_dec_float_50 float50(value_);
    float50 /= std::pow(10, precision_);
    return float50.str(precision_, std::ios_base::fixed);
  }
}