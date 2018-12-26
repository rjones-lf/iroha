/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_PEER_BUILDER_HPP
#define IROHA_PEER_BUILDER_HPP

#include "interfaces/common_objects/peer.hpp"
#include "module/shared_model/builders/common_objects/common.hpp"

// TODO: 14.02.2018 nickaleks Add check for uninitialized fields IR-972

namespace shared_model {
  namespace builder {

    /**
     * PeerBuilder is a class, used for construction of Peer objects
     * @tparam BuilderImpl is a type, which defines builder for implementation
     * of shared_model. Since we return abstract classes, it is necessary for
     * them to be instantiated with some concrete implementation
     * @tparam Validator is a type, whose responsibility is
     * to perform stateless validation on model fields
     */
    template <typename BuilderImpl, typename Validator>
    class DEPRECATED PeerBuilder
        : public CommonObjectBuilder<interface::Peer, BuilderImpl, Validator> {
     public:
      PeerBuilder address(const interface::types::AddressType &address) {
        PeerBuilder copy(*this);
        copy.builder_ = this->builder_.address(address);
        return copy;
      }

      PeerBuilder pubkey(const interface::types::PubkeyType &key) {
        PeerBuilder copy(*this);
        copy.builder_ = this->builder_.pubkey(key);
        return copy;
      }

     protected:
      virtual std::string builderName() const override {
        return "Peer Builder";
      }

      virtual validation::ReasonsGroupType validate(
          const interface::Peer &object) override {
        validation::ReasonsGroupType reasons;
        this->validator_.validatePeer(reasons, object);

        return reasons;
      }
    };
  }  // namespace builder
}  // namespace shared_model

#endif  // IROHA_PEER_BUILDER_HPP
