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

#ifndef IROHA_BUILDERS_COMMON_HPP
#define IROHA_BUILDERS_COMMON_HPP

#include "common/result.hpp"
#include "utils/polymorphic_wrapper.hpp"
#include "validators/answer.hpp"

// TODO: 16.02.2018 nickaleks: Add validators for common_objects IR-986

namespace shared_model {
  namespace builder {

    /**
     * BuilderResult represents return value of a builder,
     * it either contains a value of ModelType, an error,
     * which indicates why construction of an object went wrong.
     *
     * Since ModelType is an abstract class, its value has to be wrapped
     * in something which has polymorphic behaviour, such as PolymorphicWrapper
     */
    template <typename ModelType>
    using BuilderResult =
        iroha::expected::PolymorphicResult<ModelType, std::string>;


    /**
     * CommonObjectBuilder is a base class for all builders of common objects.
     * It encapsulates common logic
     * @tparam ModelType - type of object to be built
     * @tparam BuilderImpl - underlying implementation
     * @tparam Validator - validation object
     */
    template <typename ModelType, typename BuilderImpl, typename Validator>
    class CommonObjectBuilder {
     public:

      /**
       * build() constructs specified object and performs stateless validation on fields.
       * @return Result which contains either object, or error with explanation,
       * why object construction is unsuccessful.
       */
      BuilderResult<ModelType> build() {
        auto model_impl = std::shared_ptr<ModelType>(builder_.build().copy());

        shared_model::validation::ReasonsGroupType reasons(
            builderName(), shared_model::validation::GroupedReasons());

        validate(reasons, *model_impl);

        shared_model::validation::Answer answer;
        if (!reasons.second.empty()) {
          answer.addReason(std::move(reasons));
        }

        if (answer) {
          return iroha::expected::makeError(
              std::make_shared<std::string>(answer.reason()));
        }

        return iroha::expected::makeValue(std::move(model_impl));
      }

     protected:
      virtual std::string builderName() const = 0;

      /**
       * Perform stateless validation on an object
       * @param reasons - list of reasons, which will be populated by validator
       * @param object to be validated
       */
      virtual void validate(validation::ReasonsGroupType &reasons,
                            const ModelType &object) = 0;

      Validator validator_;
      BuilderImpl builder_;
    };
  }  // namespace builder
}  // namespace shared_model
#endif  // IROHA_BUILDERS_COMMON_HPP
