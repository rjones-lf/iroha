/*
Copyright Soramitsu Co., Ltd. 2016 All Rights Reserved.
http://soramitsu.co.jp

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

         http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#ifndef IROHAD_CONSENSUS_SUMERAGI_HPP
#define IROHAD_CONSENSUS_SUMERAGI_HPP

#include <memory>
#include <thread>
#include <vector>
#include <consensus/consensus_algorithm.hpp>

#include <block.pb.cc>

namespace consensus {
  /**
   * Sumeragi consensus algorithm
   * consensus algorithm should derive interface ConsensusAlgorithm
   */
  class Sumeragi : public ConsensusAlgorithm {
  public:
    /**
     * Event is triggered when proposal arrives.
     */
    void onProposal(iroha::dao::Proposal const& proposal) override;

    /**
     * Event is triggered when commit block arrives.
     */
    void onCommit(iroha::dao::Block const& block) override;
  };
}
#endif  // IROHAD_CONSENSUS_SUMERAGI_HPP
