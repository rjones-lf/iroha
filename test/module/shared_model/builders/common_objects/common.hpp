/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_BUILDERS_COMMON_HPP
#define IROHA_BUILDERS_COMMON_HPP

#include "common/result.hpp"
#include "utils/swig_keyword_hider.hpp"
#include "validators/answer.hpp"

// TODO: 16.02.2018 nickaleks: Add validators for common_objects IR-986

namespace shared_model {
  namespace builder {

    /**
     * BuilderResult represents return value of a builder,
     * it either contains a value of ModelType, an error,
     * which indicates why construction of an object went wrong.
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
    class DEPRECATED CommonObjectBuilder {
     public:
      /**
       * build() constructs specified object and performs stateless validation
       * on fields.
       * @return Result which contains either object, or error with explanation,
       * why object construction is unsuccessful.
       */
      BuilderResult<ModelType> build() {
        std::shared_ptr<ModelType> model_impl =
            std::move(clone(builder_.build()));

        auto reasons = validate(*model_impl);
        reasons.first = builderName();

        shared_model::validation::Answer answer;
        if (not reasons.second.empty()) {
          answer.addReason(std::move(reasons));
        }

        if (answer) {
          // TODO 15.04.2018 x3medima17 IR-1240: rework with std::string instead
          // of pointer to string
          return iroha::expected::makeError(
              std::make_shared<std::string>(answer.reason()));
        }

        return iroha::expected::makeValue(std::move(model_impl));
      }

     protected:
      /**
       * @return string name of the builder, used in result error message
       */
      virtual std::string builderName() const = 0;

      /**
       * Perform stateless validation on an object
       * @param reasons - list of reasons, which will be populated by validator
       * @param object to be validated
       */
      virtual shared_model::validation::ReasonsGroupType validate(
          const ModelType &object) = 0;

      Validator validator_;
      BuilderImpl builder_;
    };
  }  // namespace builder
}  // namespace shared_model
#endif  // IROHA_BUILDERS_COMMON_HPP
