/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
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
