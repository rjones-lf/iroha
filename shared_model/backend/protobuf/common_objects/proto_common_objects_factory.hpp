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
#include "validators/answer.hpp"

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
        auto proto_peer = std::make_unique<Peer>(std::move(peer));

        shared_model::validation::Answer answer;

        validation::ReasonsGroupType reasons;
        this->validator_.validatePeer(reasons, *proto_peer);

        if (not reasons.second.empty()) {
          answer.addReason(std::move(reasons));
        }

        if (answer) {
          return iroha::expected::makeError(answer.reason());
        }

        return iroha::expected::makeValue<std::unique_ptr<interface::Peer>>(
            std::move(proto_peer));
      }

     private:
      Validator validator_;
    };
  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_PROTO_COMMON_OBJECTS_FACTORY_HPP
