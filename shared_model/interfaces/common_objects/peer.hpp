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

#ifndef IROHA_SHARED_MODEL_PEER_HPP
#define IROHA_SHARED_MODEL_PEER_HPP

#include "interfaces/base/model_primitive.hpp"
#include "interfaces/common_objects/types.hpp"

namespace shared_model {
  namespace interface {

    /**
     * Representation of a network participant.
     */
    class Peer : public ModelPrimitive<Peer> {
     public:
      /**
       * @return Peer address, for fetching data
       */
      virtual const interface::types::AddressType &address() const = 0;

      /**
       * @return Public key, for fetching data
       */
      virtual const interface::types::PubkeyType &pubkey() const = 0;

      /**
       * Stringify the data.
       * @return the content of account asset.
       */
      std::string toString() const override {
        return detail::PrettyStringBuilder()
            .init("Peer")
            .append("address", address())
            .append("pubkey", pubkey().toString())
            .finalize();
      }

      /**
       * Checks equality of objects inside
       * @param rhs - other wrapped value
       * @return true, if wrapped objects are same
       */
      bool operator==(const ModelType &rhs) const override {
        return address() == rhs.address() and pubkey() == rhs.pubkey();
      }
    };
  }  // namespace interface
}  // namespace shared_model

#endif  // IROHA_SHARED_MODEL_PEER_HPP
