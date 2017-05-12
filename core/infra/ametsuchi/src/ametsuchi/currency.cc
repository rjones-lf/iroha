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

#include <ametsuchi/ametsuchi.h>
#include <ametsuchi/currency.h>
#include <string>

#define AMETSUCHI_MAX_PRECISION 18

namespace ametsuchi {


Currency::Currency(__int128_t amount, uint8_t precision)
    : amount_(amount), precision_(precision), div_(1) {
  // 2^64 = 1.8 * 10^19
  if (precision > AMETSUCHI_MAX_PRECISION) throw std::bad_alloc();

  for (uint8_t i = 0; i < precision_; i++) div_ *= 10;

  integer_ = amount_ / div_;
  fractional_ = amount_ % div_;
}

Currency Currency::operator+(const Currency &a) {
  return Currency(amount_ + a.amount_, precision_);
}

Currency Currency::operator-(const Currency &a) {
  return Currency(amount_ - a.amount_, precision_);
}

bool Currency::operator<(const Currency &a) {
  return (integer_ < a.integer_) ||
         (integer_ == a.integer_ && fractional_ < a.fractional_);
}

bool Currency::operator>(const Currency &a) {
  return (integer_ > a.integer_) ||
         (integer_ == a.integer_ && fractional_ > a.fractional_);
}

std::string Currency::to_string() {
  return to_string(this->integer()) + "." +
         to_string(this->fractional());
}
std::string Currency::to_string(__int128_t x){
  std::string res = "";
  bool mf = (x<0);
  while( x ) {
    res += (char)((x%10)+'0');
    x /= 10;
  }
  if( mf ) res += "-";
  reverse( res.begin(), res.end() );
  return res;
}
__int128_t Currency::integer() const { return integer_; }

__int128_t Currency::fractional() const { return fractional_; }

__int128_t Currency::get_amount() const { return amount_; }

uint8_t Currency::get_precision() const { return precision_; }

bool Currency::operator==(const Currency &a) {
  return this->integer_ == a.integer_ && this->fractional_ == a.fractional_;
}

}  // namespace ametsuchi
