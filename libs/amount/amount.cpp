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

#include "amount/amount.hpp"

#include <regex>
#include <utility>

using namespace boost::multiprecision;

namespace iroha {

  // to raise to power integer values
  static int ipow(int base, int exp) {
    int result = 1;
    while (exp != 0) {
      if (exp & 1) {
        result *= base;
      }
      exp >>= 1;
      base *= base;
    }

    return result;
  }

  static uint256_t getJointUint256(uint64_t first,
                                   uint64_t second,
                                   uint64_t third,
                                   uint64_t fourth) {
    // join 4 uint64_t into single uint256_t by means of logic or operator
    uint256_t res(0);
    res |= first;
    res <<= 64;
    res |= second;
    res <<= 64;
    res |= third;
    res <<= 64;
    res |= fourth;
    return res;
  }

  Amount::Amount() {}

  Amount::Amount(uint256_t value) : value_(value) {}

  Amount::Amount(uint256_t amount, uint8_t precision)
      : value_(amount), precision_(precision) {}

  Amount::Amount(uint64_t first,
                 uint64_t second,
                 uint64_t third,
                 uint64_t fourth)
      : Amount(first, second, third, fourth, 0) {}

  Amount::Amount(uint64_t first,
                 uint64_t second,
                 uint64_t third,
                 uint64_t fourth,
                 uint8_t precision)
      : precision_(precision) {
    value_ = getJointUint256(first, second, third, fourth);
  }

  Amount::Amount(const Amount &am)
      : value_(am.value_), precision_(am.precision_) {}

  Amount &Amount::operator=(const Amount &other) {
    // check for self-assignment
    if (&other == this)
      return *this;
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

  nonstd::optional<Amount> Amount::createFromString(std::string str_amount) {
    // check if valid number
    std::regex e("([0-9]*\\.[0-9]+|[0-9]+)");
    if (!std::regex_match(str_amount, e)) {
      return nonstd::nullopt;
    }

    // get precision
    auto dot_place = str_amount.find('.');
    size_t precision;
    if (dot_place > str_amount.size()) {
      precision = 0;
    } else {
      precision = str_amount.size() - dot_place - 1;
      // erase dot from the string
      str_amount.erase(std::remove(str_amount.begin(), str_amount.end(), '.'),
                       str_amount.end());
    }

    auto begin = str_amount.find_first_not_of('0');

    // create uint256 value from obtained string
    uint256_t value = 0;
    if (begin <= str_amount.size()) {
      value = uint256_t(str_amount.substr(begin));
    }
    return Amount(value, precision);
  }

  uint256_t Amount::getIntValue() {
    return value_;
  }

  uint8_t Amount::getPrecision() {
    return precision_;
  }

  std::vector<uint64_t> Amount::to_uint64s() {
    std::vector<uint64_t> array(4);
    ;
    for (int i = 0; i < 4; i++) {
      uint64_t res = (value_ >> i * 64).convert_to<uint64_t>();
      array[3 - i] = res;
    }
    return array;
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

  Amount Amount::subtract(const Amount &other) const {
    auto new_val = value_ - other.value_;
    return {new_val, precision_};
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
    if (precision_ > 0) {
      cpp_dec_float_50 float50(value_);
      float50 /= pow(10, precision_);
      return float50.str(precision_, std::ios_base::fixed);
    }
    return value_.str(0, std::ios_base::fixed);
  }
}  // namespace iroha
