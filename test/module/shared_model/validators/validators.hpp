/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_VALIDATOR_MOCKS_HPP
#define IROHA_VALIDATOR_MOCKS_HPP

#include "validators/abstract_validator.hpp"

#include <gmock/gmock.h>

namespace shared_model {
  namespace validation {

    // TODO: kamilsa 01.02.2018 IR-873 Replace all these validators with mock
    // classes

    struct AlwaysValidValidator {
      template <typename T>
      Answer validate(const T &) const {
        return {};
      }
    };

    template <typename T>
    class MockValidator : public AbstractValidator<T> {
     public:
      MOCK_CONST_METHOD1_T(validate, Answer(const T &));
    };

  }  // namespace validation
}  // namespace shared_model

#endif  // IROHA_VALIDATOR_MOCKS_HPP
