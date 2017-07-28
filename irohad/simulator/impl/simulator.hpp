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

#ifndef IROHA_SIMULATOR_HPP
#define IROHA_SIMULATOR_HPP

#include "ametsuchi/block_query.hpp"
#include "ametsuchi/temporary_factory.hpp"
#include "model/model_hash_provider_impl.hpp"
#include "simulator/block_creator.hpp"
#include "simulator/verified_proposal_creator.hpp"
#include "validation/stateful_validator.hpp"

namespace iroha {
  namespace simulator {

    class Simulator : public VerifiedProposalCreator, BlockCreator {
     public:
      Simulator(validation::StatefulValidator& statefulValidator,
                ametsuchi::TemporaryFactory& factory,
                ametsuchi::BlockQuery& blockQuery,
                model::HashProviderImpl& hash_provider);

      void process_proposal(model::Proposal proposal) override;

      rxcpp::observable<nonstd::optional<model::Proposal>>
      on_verified_proposal() override;

      void process_verified_proposal(
          nonstd::optional<model::Proposal> proposal) override;

      rxcpp::observable<nonstd::optional<model::Block>> on_block() override;

     private:
      // internal
      rxcpp::subjects::subject<nonstd::optional<model::Proposal>> notifier_;
      rxcpp::subjects::subject<nonstd::optional<model::Block>> block_notifier_;

      validation::StatefulValidator& validator_;
      ametsuchi::TemporaryFactory& ametsuchi_factory_;
      ametsuchi::BlockQuery& block_queries_;
      model::HashProviderImpl& hash_provider_;

      // last block
      model::Block last_block;
    };
  }  // namespace simulator
}  // namespace iroha

#endif  // IROHA_SIMULATOR_HPP
