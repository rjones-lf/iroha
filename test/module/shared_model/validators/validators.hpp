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

    struct AlwaysValidFieldValidator final {
      template <typename... Args>
      void validateAccountId(Args...) const {}
      template <typename... Args>
      void validateAssetId(Args...) const {}
      template <typename... Args>
      void validatePeer(Args...) const {}
      template <typename... Args>
      void validateAmount(Args...) const {}
      template <typename... Args>
      void validatePubkey(Args...) const {}
      template <typename... Args>
      void validatePeerAddress(Args...) const {}
      template <typename... Args>
      void validateRoleId(Args...) const {}
      template <typename... Args>
      void validateAccountName(Args...) const {}
      template <typename... Args>
      void validateDomainId(Args...) const {}
      template <typename... Args>
      void validateAssetName(Args...) const {}
      template <typename... Args>
      void validateAccountDetailKey(Args...) const {}
      template <typename... Args>
      void validateAccountDetailValue(Args...) const {}
      template <typename... Args>
      void validatePrecision(Args...) const {}
      template <typename... Args>
      void validateRolePermission(Args...) const {}
      template <typename... Args>
      void validateGrantablePermission(Args...) const {}
      template <typename... Args>
      void validateQuorum(Args...) const {}
      template <typename... Args>
      void validateCreatorAccountId(Args...) const {}
      template <typename... Args>
      void validateCreatedTime(Args...) const {}
      template <typename... Args>
      void validateCounter(Args...) const {}
      template <typename... Args>
      void validateSignatures(Args...) const {}
      template <typename... Args>
      void validateQueryPayloadMeta(Args...) const {}
      template <typename... Args>
      void validateDescription(Args...) const {}
      template <typename... Args>
      void validateBatchMeta(Args...) const {}
      template <typename... Args>
      void validateHeight(Args...) const {}
      template <typename... Args>
      void validateHash(Args...) const {}
    };

    template <typename Model>
    struct AlwaysValidModelValidator final : public AbstractValidator<Model> {
     public:
      Answer validate(const Model &m) const override{return {};};
    };

  }  // namespace validation
}  // namespace shared_model

#endif  // IROHA_VALIDATOR_MOCKS_HPP
