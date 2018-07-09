/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_PROTO_COMMON_OBJECTS_FACTORY_HPP
#define IROHA_PROTO_COMMON_OBJECTS_FACTORY_HPP

#include "interfaces/common_objects/common_objects_factory.hpp"

namespace shared_model {
  namespace proto {
    class ProtoCommonObjectsFactory : public interface::CommonObjectsFactory {
     public:
      std::unique_ptr<interface::Peer> createPeer(
          const interface::types::AddressType &address,
          const interface::types::PubkeyType &public_key) override;
    };
  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_PROTO_COMMON_OBJECTS_FACTORY_HPP
