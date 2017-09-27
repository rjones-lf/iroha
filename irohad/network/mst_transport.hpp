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

#ifndef IROHA_MST_TRANSPORT_HPP
#define IROHA_MST_TRANSPORT_HPP

#include "multi_sig_transactions/storage/mst_state.hpp"
#include "model/peer.hpp"
#include "memory"

namespace iroha {
  namespace network {

    /**
     * Interface represents handler for multi-signature notifications
     */
    class MstTransportNotification {
     public:

      /**
       * Handler method for updating state, when new data received
       * @param from - peer emitter of state
       * @param new_state - state propagated from peer
       */
      virtual void onStateUpdate(model::Peer from, MstState new_state) = 0;

      virtual ~MstTransportNotification() = default;
    };

    /**
     * Interface of transport for propagating multi-signature transactions in network
     */
    class MstTransport {

      /**
       * Subscribe object for receiving notifications
       * @param notification - object that will be notified on updates
       */
      virtual void subscribe(std::shared_ptr<MstTransportNotification> notification) = 0;

      /**
       * Share state with other peer
       * @param to - peer recipient of message
       * @param providing_state - state for transmitting
       */
      virtual void sendState(model::Peer to, MstState providing_state) = 0;

      virtual ~MstTransport() = default;
    };
  }
} // namespace iroha // namespace network
#endif //IROHA_MST_TRANSPORT_HPP
