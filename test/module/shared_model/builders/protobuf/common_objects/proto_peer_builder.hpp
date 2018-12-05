/**
 * Copyright Soramitsu Co., Ltd. 2018 All Rights Reserved.
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

#ifndef IROHA_PROTO_PEER_BUILDER_HPP
#define IROHA_PROTO_PEER_BUILDER_HPP

#include "backend/protobuf/common_objects/peer.hpp"
#include "primitive.pb.h"

namespace shared_model {
  namespace proto {

    /**
     * PeerBuilder is used to construct Peer proto objects with initialized
     * protobuf implementation
     */
    class DEPRECATED PeerBuilder {
     public:
      shared_model::proto::Peer build() {
        return shared_model::proto::Peer(iroha::protocol::Peer(peer_));
      }

      PeerBuilder address(const interface::types::AddressType &address) {
        PeerBuilder copy(*this);
        copy.peer_.set_address(address);
        return copy;
      }

      PeerBuilder pubkey(const interface::types::PubkeyType &key) {
        PeerBuilder copy(*this);
        copy.peer_.set_peer_key(key.hex());
        return copy;
      }

     private:
      iroha::protocol::Peer peer_;
    };
  }  // namespace proto
}  // namespace shared_model
#endif  // IROHA_PROTO_PEER_BUILDER_HPP
