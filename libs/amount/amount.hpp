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
#include <boost/multiprecision/cpp_int.hpp>
#include <cstdint>
#include <string>

namespace amount {

  /**
   * Keeps integer and scale values allowing performing math
   * operations on them
   */
  class Amount {
   public:
    /**
     * Creates Amount with integer = 0 and scale = 0
     */
    Amount();

    /**
     * Amount with integer = amount and scale = 0
     * @param amount integer part
     */
    Amount(boost::multiprecision::uint256_t amount);

    /**
     * Amount with provided integer and scale part
     * @param amount integer part
     * @param precision scale part
     */
    Amount(boost::multiprecision::uint256_t amount, uint8_t precision);

    /**
     * Copy constructor
     */
    Amount(const Amount&);
    Amount& operator=(const Amount&);

    /**
     * Move constructor
     */
    Amount(Amount&&);
    Amount& operator=(Amount&&);

    /**
     * Takes percentage from current amount
     * @param percents
     * @return
     */
    Amount percentage(uint64_t percents) const;

    /**
     * Takes percentage represented as amount value
     * The current scale and scale of percents may vary
     * @param percents
     * @return
     */
    Amount percentage(const Amount& percents) const;

    /**
     * Sums two amounts. Requires to have the same scale
     * @return
     */
    Amount operator+(const Amount&) const;
    Amount& operator+=(const Amount&);

    /**
     * Subtracts one amount from another.
     * Requires to have the same scale between both amounts
     * @return
     */
    Amount operator-(const Amount&) const;
    Amount& operator-=(const Amount&);

    /**
     * Two amounts are equal, when integer and scale parts are equal
     * @return
     */
    bool operator==(const Amount&) const;
    bool operator!=(const Amount&) const;
    bool operator<(const Amount&) const;
    bool operator>(const Amount&) const;
    bool operator<=(const Amount&) const;
    bool operator>=(const Amount&) const;

    std::string to_string() const;
    ~Amount() = default;

   private:
    boost::multiprecision::uint256_t value_;
    uint8_t precision_;
  };
}
#endif  // IROHA_AMOUNT_H
