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

#ifndef IROHA_ADD_PEER_HPP
#define IROHA_ADD_PEER_HPP

#include "model/command.hpp"
#include "model/peer.hpp"

namespace iroha {
  namespace model {

    struct Peer;

    /**
     * Provide user's intent for adding peer to current network
     */
    struct AddPeer : public Command {
      Peer peer;

      bool operator==(const Command &command) const override;

      AddPeer() {}

      AddPeer(const Peer &peer) : peer(peer) {}
    };
  }  // namespace model
}  // namespace iroha
#endif  // IROHA_ADD_PEER_HPP
