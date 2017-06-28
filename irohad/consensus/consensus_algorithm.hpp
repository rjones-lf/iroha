/*
 * Copyright Soramitsu Co., Ltd. 2017 All Rights Reserved.
 * http://soramitsu.co.jp
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *          http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef IROHAD_CONSENSUS_CONSENSUS_ALGORITHM_HPP
#define IROHAD_CONSENSUS_CONSENSUS_ALGORITHM_HPP

#include <rxcpp/rx.hpp>

#include <dao/proposal.hpp>
#include <dao/block.hpp>

#include <network/network_api.h>


namespace consensus {

  class ConsensusAlgorithm {
  public:

    /**
     * Event is triggered when proposal arrives.
     */
    virtual void onProposal(iroha::dao::Proposal const& proposal) = 0;

    /**
     * Event is triggered when commit block arrives.
     */
    virtual void onCommit(iroha::dao::Block const& block) = 0;
  };

}  // namespace consensus
#endif  // IROHAD_CONSENSUS_CONSENSUS_ALGORITHM_HPP

