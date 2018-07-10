/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_PROTO_COMMON_OBJECTS_FACTORY_HPP
#define IROHA_PROTO_COMMON_OBJECTS_FACTORY_HPP

#include "backend/protobuf/common_objects/peer.hpp"
#include "common/result.hpp"
#include "interfaces/common_objects/common_objects_factory.hpp"
#include "primitive.pb.h"

namespace shared_model {
  namespace proto {
    template <typename Validator>
    class ProtoCommonObjectsFactory : public interface::CommonObjectsFactory {
     public:
      FactoryResult<std::unique_ptr<interface::Peer>> createPeer(
          const interface::types::AddressType &address,
          const interface::types::PubkeyType &public_key) override {
        iroha::protocol::Peer peer;
        peer.set_address(address);
        peer.set_peer_key(crypto::toBinaryString(public_key));
        return iroha::expected::makeValue<std::unique_ptr<interface::Peer>>(
            std::make_unique<Peer>(std::move(peer)));
      }

     private:
      Validator validator;
    };
  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_PROTO_COMMON_OBJECTS_FACTORY_HPP
