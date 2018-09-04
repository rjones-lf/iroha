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

#ifndef IROHA_YAC_NETWORK_INTERFACE_HPP
#define IROHA_YAC_NETWORK_INTERFACE_HPP

#include <memory>
#include <vector>

namespace shared_model {
  namespace interface {
    class Peer;
  }  // namespace interface
}  // namespace shared_model

namespace iroha {
  namespace consensus {
    namespace yac {

      struct VoteMessage;

      class YacNetworkNotifications {
       public:
        /**
         * Callback on receiving collection of votes
         * @param state - provided message
         */
        virtual void onState(std::vector<VoteMessage> state) = 0;

        virtual ~YacNetworkNotifications() = default;
      };

      class YacNetwork {
       public:
        virtual void subscribe(
            std::shared_ptr<YacNetworkNotifications> handler) = 0;

        /**
         * Directly share collection of votes
         * @param to - peer recipient
         * @param state - message for sending
         */
        virtual void sendState(const shared_model::interface::Peer &to,
                               const std::vector<VoteMessage> &state) = 0;

        /**
         * Virtual destructor required for inheritance
         */
        virtual ~YacNetwork() = default;
      };
    }  // namespace yac
  }    // namespace consensus
}  // namespace iroha

#endif  // IROHA_YAC_NETWORK_INTERFACE_HPP
