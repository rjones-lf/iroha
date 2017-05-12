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

#ifndef AMETSUCHI_CURRENCY_H
#define AMETSUCHI_CURRENCY_H

#include <bits/stdc++.h>
#include <stdint.h>
#include <string>

namespace ametsuchi {

class Currency {
 public:
  explicit Currency(__int128_t amount, uint8_t precision = 2);

  Currency operator+(const Currency &a);
  Currency operator-(const Currency &a);
  bool operator==(const Currency &a);
  bool operator<(const Currency &a);
  bool operator>(const Currency &a);

  __int128_t integer() const;
  __int128_t fractional() const;
  __int128_t get_amount() const;
  uint8_t get_precision() const;

  std::string to_string();
  std::string to_string(__int128_t x);

 private:
  __int128_t amount_;
  uint8_t precision_;
  __int128_t div_;
  __int128_t integer_;
  __int128_t fractional_;
};
}  // namespace ametsuchi

#endif  // AMETSUCHI_CURRENCY_H
