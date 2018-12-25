/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_ABSTRACT_VALIDATOR_HPP
#define IROHA_ABSTRACT_VALIDATOR_HPP

#include <regex>
#include "validators/answer.hpp"

namespace shared_model {
  namespace validation {

    // validator which can be overloaded for dynamic polymorphism
    template <typename Model>
    class AbstractValidator {
     protected:
      bool validateHexString(const std::string &str) const {
        static const std::regex hex_regex{R"([0-9a-fA-F]*)"};
        return std::regex_match(str, hex_regex);
      }

     public:
      virtual Answer validate(const Model &m) const = 0;

      virtual ~AbstractValidator() = default;
    };

  }  // namespace validation
}  // namespace shared_model

#endif  // IROHA_ABSTRACT_VALIDATOR_HPP
