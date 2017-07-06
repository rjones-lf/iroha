/*
Copyright Soramitsu Co., Ltd. 2016 All Rights Reserved.

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
#ifndef IROHA_NETWORK_H
#define IROHA_NETWORK_H

#include "dao/dao.hpp"
#include "rxcpp/rx-observable.hpp"

namespace iroha {
  namespace network {

    /**
     * Interface provides methods for fetching consensus-related data.
     */
    class ConsensusListener {
     public:
      /**
       * Event is triggered when proposal arrives from network.
       * @return observable with Proposals.
       * (List of Proposals)
       */
      virtual rxcpp::observable<dao::Proposal> on_proposal() = 0;

      /**
       * Event is triggered when commit block arrives.
       * @return observable with sequence of committed blocks.
       * In common case observable<Block> will contain one element.
       * But there are scenarios when consensus provide many blocks, e.g.
       * on peer startup - peer will get all actual blocks.
       */
      virtual rxcpp::observable<rxcpp::observable<dao::Block>> on_commit() = 0;
    };

    /**
     * Interface for propagating transaction in a network
     */
    class TransactionPropagator {
     public:
      /**
       * Method spreads transaction to other members of a network
       * @param tx - transaction for propagation
       */
      virtual void propagate_transaction(const dao::Transaction &tx) = 0;
    };

    /**
     * Public API interface for communication between current peer and other
     * peers in a network
     */
    class PeerCommunicationService : public TransactionPropagator,
                                     public ConsensusListener {
    };
  }
}
#endif  // IROHA_NETWORK_H
