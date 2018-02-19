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

#ifndef IROHA_PEER_BUILDER_HPP
#define IROHA_PEER_BUILDER_HPP

#include "builders/common_objects/common.hpp"
#include "interfaces/common_objects/peer.hpp"

//TODO: 14.02.2018 nickaleks Add check for uninitialized fields IR-972

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
    class PeerBuilder: public CommonObjectBuilder<interface::Peer,
                                                  BuilderImpl,
                                                  Validator> {
     public:

      PeerBuilder &address(const interface::types::AddressType &address) {
        this->builder_ = this->builder_.address(address);
        return *this;
      }

      PeerBuilder &pubkey(const interface::types::PubkeyType &key) {
        this->builder_ = this->builder_.pubkey(key);
        return *this;
      }

     protected:
      virtual std::string builderName() const override {
        return "Peer Builder";
      }

      virtual void validate(validation::ReasonsGroupType &reasons,
                            const interface::Peer &object) override {
        this->validator_.validatePeer(reasons, object);
      }
    };
  }  // namespace builder
}  // namespace shared_model

#endif  // IROHA_PEER_BUILDER_HPP
