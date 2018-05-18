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

#ifndef IROHA_VERIFIED_PROPOSAL_CREATOR_HPP
#define IROHA_VERIFIED_PROPOSAL_CREATOR_HPP

#include <rxcpp/rx-observable.hpp>

namespace shared_model {
  namespace interface {
    class Proposal;
    class VerifiedProposal;
  }  // namespace interface
}  // namespace shared_model

namespace iroha {
  namespace simulator {

    /**
     * Interface for providing proposal validation
     */
    class VerifiedProposalCreator {
     public:
      /**
       * Processing proposal for making stateful validation
       * @param proposal - object for validation
       */
      virtual void process_proposal(
          const shared_model::interface::Proposal &proposal) = 0;

      /**
       * Emit proposals that was verified by validation
       * @return
       */
      virtual rxcpp::observable<
          std::shared_ptr<shared_model::interface::VerifiedProposal>>
      on_verified_proposal() = 0;

      virtual ~VerifiedProposalCreator() = default;
    };
  }  // namespace simulator
}  // namespace iroha
#endif  // IROHA_VERIFIED_PROPOSAL_CREATOR_HPP
