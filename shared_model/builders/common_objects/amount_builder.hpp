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

#include "builders/common_objects/common.hpp"
#include "interfaces/common_objects/amount.hpp"

//TODO: 14.02.2018 nickaleks Add check for uninitialized fields IR-972

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
    class AmountBuilder {
     public:
      BuilderResult<shared_model::interface::Amount> build() {
        auto amount = builder_.build();
        shared_model::validation::ReasonsGroupType reasons(
            "Amount Builder", shared_model::validation::GroupedReasons());
        shared_model::validation::Answer answer;
        validator_.validateAmount(reasons, amount);

        if (!reasons.second.empty()) {
          answer.addReason(std::move(reasons));
          return iroha::expected::makeError(
              std::make_shared<std::string>(answer.reason()));
        }
        std::shared_ptr<shared_model::interface::Amount> amount_ptr(amount.copy());
        return iroha::expected::makeValue(amount_ptr);
      }

      AmountBuilder &intValue(const boost::multiprecision::uint256_t &value) {
        builder_ = builder_.intValue(value);
        return *this;
      }

      AmountBuilder &precision(const interface::types::PrecisionType &precision) {
        builder_ = builder_.precision(precision);
        return *this;
      }

     private:
      Validator validator_;
      BuilderImpl builder_;
    };
  }  // namespace builder
}  // namespace shared_model

#endif  // IROHA_AMOUNT_BUILDER_HPP
