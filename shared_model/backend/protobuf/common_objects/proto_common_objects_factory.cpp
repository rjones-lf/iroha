/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "proto_common_objects_factory.hpp"

#include "backend/protobuf/common_objects/peer.hpp"

namespace shared_model {
  namespace proto {
    std::unique_ptr<interface::Peer> ProtoCommonObjectsFactory::createPeer(
        const interface::types::AddressType &address,
        const interface::types::PubkeyType &public_key) {
      iroha::protocol::Peer peer;
      peer.set_address (address);
      peer.set_peer_key(crypto::toBinaryString(public_key));
      return std::make_unique<Peer>(std::move(peer));
    }
  }  // namespace proto
}  // namespace shared_model
