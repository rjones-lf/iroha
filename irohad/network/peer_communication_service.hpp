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

namespace shared_model {
  namespace interface {
    class Block;
    class Proposal;
    class Transaction;
  }  // namespace interface
}  // namespace shared_model

namespace iroha {

  using Commit =
      rxcpp::observable<std::shared_ptr<shared_model::interface::Block>>;

  namespace network {

    /**
     * Public API for notification about domain data
     */
    class PeerCommunicationService {
     public:
      /**
       * Propagate transaction in network
       * @param transaction - object for propagation
       */
      virtual void propagate_transaction(
          std::shared_ptr<const shared_model::interface::Transaction>
              transaction) = 0;

      /**
       * Event is triggered when proposal arrives from network.
       * @return observable with Proposals.
       * (List of Proposals)
       */
      virtual rxcpp::observable<
          std::shared_ptr<shared_model::interface::Proposal>>
      on_proposal() = 0;

      /**
       * Event is triggered when commit block arrives.
       * @return observable with sequence of committed blocks.
       * In common case observable<Block> will contain one element.
       * But there are scenarios when consensus provide many blocks, e.g.
       * on peer startup - peer will get all actual blocks.
       */
      virtual rxcpp::observable<Commit> on_commit() = 0;

      virtual ~PeerCommunicationService() = default;
    };
  }  // namespace network
}  // namespace iroha
#endif  // IROHA_PEER_COMMUNICATION_SERVICE_HPP
