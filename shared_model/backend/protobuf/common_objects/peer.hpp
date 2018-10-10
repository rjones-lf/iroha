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

#ifndef IROHA_SHARED_MODEL_PROTO_PEER_HPP
#define IROHA_SHARED_MODEL_PROTO_PEER_HPP

#include "interfaces/common_objects/peer.hpp"

#include "backend/protobuf/common_objects/trivial_proto.hpp"
#include "backend/protobuf/util.hpp"
#include "cryptography/public_key.hpp"
#include "primitive.pb.h"
#include "utils/lazy_initializer.hpp"

namespace shared_model {
  namespace proto {
    class Peer final
        : public CopyableProto<interface::Peer, iroha::protocol::Peer, Peer> {
     public:
      template <typename PeerType>
      explicit Peer(PeerType &&peer)
          : CopyableProto(std::forward<PeerType>(peer)) {}

      Peer(const Peer &o) : Peer(o.proto_) {}

      Peer(Peer &&o) noexcept : Peer(std::move(o.proto_)) {}

      const interface::types::AddressType &address() const override {
        return proto_->address();
      }

      const interface::types::PubkeyType &pubkey() const override {
        return *public_key_;
      }

     private:
      // lazy
      template <typename T>
      using Lazy = detail::LazyInitializer<T>;

      const Lazy<interface::types::PubkeyType> public_key_{
          [this] { return interface::types::PubkeyType(
                  crypto::Blob::fromHexString(proto_->peer_key())); }};
    };

  }  // namespace proto
}  // namespace shared_model
#endif  // IROHA_SHARED_MODEL_PROTO_PEER_HPP
