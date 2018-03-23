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

#ifndef IROHA_AMOUNT_BUILDER_HPP
#define IROHA_AMOUNT_BUILDER_HPP

#include <regex>

#include "builders/common_objects/common.hpp"
#include "interfaces/common_objects/amount.hpp"

// TODO: 14.02.2018 nickaleks Add check for uninitialized fields IR-972

namespace shared_model {
  namespace builder {

    /**
     * AmountBuilder is a class, used for construction of Amount objects
     * @tparam BuilderImpl is a type, which defines builder for implementation
     * of shared_model. Since we return abstract classes, it is necessary for
     * them to be instantiated with some concrete implementation
     * @tparam Validator is a type, whose responsibility is
     * to perform stateless validation on model fields
     */
    template <typename BuilderImpl, typename Validator>
    class AmountBuilder : public CommonObjectBuilder<interface::Amount,
                                                     BuilderImpl,
                                                     Validator> {
     public:
      AmountBuilder intValue(const boost::multiprecision::uint256_t &value) {
        AmountBuilder copy(*this);
        copy.builder_ = this->builder_.intValue(value);
        return copy;
      }

      AmountBuilder precision(
          const interface::types::PrecisionType &precision) {
        AmountBuilder copy(*this);
        copy.builder_ = this->builder_.precision(precision);
        return copy;
      }

      /**
       * Constructs Amount object from given string
       * @param str_amount string in format "100.00",
       * where there are exactly precision numbers after dot
       * @return Amount constructed from string
       */
      static BuilderResult<shared_model::interface::Amount> fromString(
          std::string str_amount) {
        // taken from iroha::model::Amount
        // check if valid number
        std::regex e("([0-9]*\\.[0-9]+|[0-9]+)");
        if (!std::regex_match(str_amount, e)) {
          return iroha::expected::makeError(
              std::make_shared<std::string>("number string is invalid"));
        }

        // get precision
        auto dot_place = str_amount.find('.');
        size_t precision;
        if (dot_place > str_amount.size()) {
          precision = 0;
        } else {
          precision = str_amount.size() - dot_place - 1;
          // erase dot from the string
          str_amount.erase(
              std::remove(str_amount.begin(), str_amount.end(), '.'),
              str_amount.end());
        }

        auto begin = str_amount.find_first_not_of('0');

        // create uint256 value from obtained string
        boost::multiprecision::uint256_t value = 0;
        if (begin <= str_amount.size()) {
          value = boost::multiprecision::uint256_t(str_amount.substr(begin));
        }

        AmountBuilder<BuilderImpl, Validator> builder;
        return builder.intValue(value).precision(precision).build();
      }

     protected:
      virtual std::string builderName() const override {
        return "Amount Builder";
      }

      virtual validation::ReasonsGroupType validate(
          const interface::Amount &object) override {
        validation::ReasonsGroupType reasons;
        this->validator_.validateAmount(reasons, object);

        return reasons;
      }
    };
  }  // namespace builder
}  // namespace shared_model

#endif  // IROHA_AMOUNT_BUILDER_HPP
