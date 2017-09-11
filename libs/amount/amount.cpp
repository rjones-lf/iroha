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

namespace amount {

  Amount::Amount() {}

  Amount::Amount(uint64_t amount, uint8_t precision) {
    value = amount;
    value /= std::pow(10, precision);
  }
  Amount::Amount(uint64_t amount) { value = amount; }
  Amount::Amount(const Amount &) {}
  Amount &Amount::operator=(const Amount &) { return *this; }
  Amount::Amount(Amount &&) {}
  Amount &Amount::operator=(Amount &&) { return *this; }
  Amount Amount::percentage(uint64_t) const { return Amount(); }
  Amount Amount::percentage(const Amount &) const { return Amount(); }
  Amount Amount::operator+(const Amount &) const { return Amount(); }
  Amount &Amount::operator+=(const Amount &) { return *this; }
  Amount Amount::operator-(const Amount &) const { return Amount(); }
  Amount &Amount::operator-=(const Amount &) { return *this; }
  bool Amount::operator==(const Amount &) const { return false; }
  bool Amount::operator!=(const Amount &) const { return false; }
  bool Amount::operator<(const Amount &) const { return false; }
  bool Amount::operator>(const Amount &) const { return false; }
  bool Amount::operator<=(const Amount &) const { return false; }
  bool Amount::operator>=(const Amount &) const { return false; }
}
}