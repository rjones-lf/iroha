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

#ifndef IROHA_VALIDATION_MOCKS_HPP
#define IROHA_VALIDATION_MOCKS_HPP

#include <gmock/gmock.h>

#include "interfaces/iroha_internal/proposal.hpp"
#include "validation/chain_validator.hpp"
#include "validation/stateful_validator.hpp"

namespace iroha {
  namespace validation {
    class MockStatefulValidator : public validation::StatefulValidator {
     public:
      MOCK_METHOD2(validate,
                   std::shared_ptr<shared_model::interface::VerifiedProposal>(
                       const shared_model::interface::Proposal &,
                       ametsuchi::TemporaryWsv &));
    };

    class MockChainValidator : public ChainValidator {
     public:
      MOCK_METHOD2(validateChain,
                   bool(rxcpp::observable<
                            std::shared_ptr<shared_model::interface::Block>>,
                        ametsuchi::MutableStorage &));

      MOCK_METHOD2(validateBlock,
                   bool(const shared_model::interface::Block &,
                        ametsuchi::MutableStorage &));
    };
  }  // namespace validation
}  // namespace iroha

#endif  // IROHA_VALIDATION_MOCKS_HPP
