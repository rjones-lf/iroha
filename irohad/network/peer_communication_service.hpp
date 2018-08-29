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

#ifndef IROHA_PEER_COMMUNICATION_SERVICE_HPP
#define IROHA_PEER_COMMUNICATION_SERVICE_HPP

#include <rxcpp/rx.hpp>

#include "synchronizer/synchronizer_common.hpp"
#include "validation/stateful_validator_common.hpp"

namespace shared_model {
  namespace interface {
    class Block;
    class Proposal;
    class Transaction;
    class TransactionBatch;
  }  // namespace interface
}  // namespace shared_model

namespace iroha {

  namespace network {

    /**
     * Public API for notification about domain data
     */
    class PeerCommunicationService {
     public:
      /**
       * @deprecated use propagate_batch instead
       * Propagate transaction in network
       * @param transaction - object for propagation
       */
      virtual void propagate_transaction(
          std::shared_ptr<const shared_model::interface::Transaction>
              transaction) const = 0;

      /**
       * Propagate batch to the network
       * @param batch - batch for propagation
       */
      virtual void propagate_batch(
          const shared_model::interface::TransactionBatch &batch) const = 0;

      /**
       * Event is triggered when proposal arrives from network.
       * @return observable with Proposals.
       * (List of Proposals)
       */
      virtual rxcpp::observable<
          std::shared_ptr<shared_model::interface::Proposal>>
      on_proposal() const = 0;

      /**
       * Event is triggered when verified proposal arrives
       * @return verified proposal and list of stateful validation errors
       */
      virtual rxcpp::observable<
          std::shared_ptr<iroha::validation::VerifiedProposalAndErrors>>
      on_verified_proposal() const = 0;

      /**
       * Event is triggered when commit block arrives.
       * @return observable with sequence of committed blocks.
       * In common case observable<Block> will contain one element.
       * But there are scenarios when consensus provide many blocks, e.g.
       * on peer startup - peer will get all actual blocks.
       * Also, it can provide no blocks at all, if commit was empty
       */
      virtual rxcpp::observable<synchronizer::SynchronizationEvent> on_commit()
          const = 0;

      virtual ~PeerCommunicationService() = default;
    };
  }  // namespace network
}  // namespace iroha
#endif  // IROHA_PEER_COMMUNICATION_SERVICE_HPP
