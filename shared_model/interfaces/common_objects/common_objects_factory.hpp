/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_COMMON_OBJECTS_FACTORY_HPP
#define IROHA_COMMON_OBJECTS_FACTORY_HPP

#include <memory>

#include "interfaces/common_objects/peer.hpp"
#include "interfaces/common_objects/types.hpp"

namespace shared_model {
  namespace interface {
    /**
     * CommonObjectsFactory provides methods to construct simple objects
     * such as peer, account etc.
     */
    class CommonObjectsFactory {
     public:
      /**
       * Creates peer instance
       * @param address - ip address of the peer
       * @param public_key - public key of the peer
       */
      virtual std::unique_ptr<Peer> createPeer(
          const types::AddressType &address,
          const types::PubkeyType &public_key) = 0;

      virtual ~CommonObjectsFactory() = default;
    };
  }  // namespace interface
}  // namespace shared_model

#endif  // IROHA_COMMONOBJECTSFACTORY_HPP
