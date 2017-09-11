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
#ifndef IROHA_AMOUNT_H
#define IROHA_AMOUNT_H

#include <boost/multiprecision/cpp_dec_float.hpp>
#include <cstdint>
#include <string>

namespace amount {

  class Amount {
   public:
    Amount();
    Amount(uint64_t amount);
    Amount(uint64_t amount, uint8_t precision);

    Amount(const Amount&);
    Amount& operator=(const Amount&);

    Amount(Amount&&);
    Amount& operator=(Amount&&);

    Amount percentage(uint64_t) const;
    Amount percentage(const Amount&) const;

    Amount operator+(const Amount&) const;
    Amount& operator+=(const Amount&);

    Amount operator-(const Amount&) const;
    Amount& operator-=(const Amount&);

    bool operator==(const Amount&) const;
    bool operator!=(const Amount&) const;
    bool operator<(const Amount&) const;
    bool operator>(const Amount&) const;
    bool operator<=(const Amount&) const;
    bool operator>=(const Amount&) const;

    std::string to_string() const;
    ~Amount() = default;

   private:
    boost::multiprecision::cpp_dec_float_50 value;
  };
}
#endif  // IROHA_AMOUNT_H
