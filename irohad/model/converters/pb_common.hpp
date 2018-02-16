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

#ifndef IROHA_PB_COMMON_HPP
#define IROHA_PB_COMMON_HPP

#include "amount/amount.hpp"
#include "commands.pb.h"
#include "common/types.hpp"
#include "cryptography/ed25519_sha3_impl/internal/sha3_hash.hpp"
#include "model/account.hpp"
#include "model/account_asset.hpp"
#include "model/asset.hpp"
#include "model/domain.hpp"
#include "model/peer.hpp"
#include "model/signature.hpp"
#include "responses.pb.h"

namespace iroha {
  namespace model {
    namespace converters {
      // amount
      protocol::Amount serializeAmount(iroha::Amount iroha_amount);
      iroha::Amount deserializeAmount(protocol::Amount pb_amount);

      // peer
      protocol::Peer serializePeer(iroha::model::Peer iroha_peer);
      iroha::model::Peer deserializePeer(protocol::Peer pb_peer);

      iroha::protocol::Account serializeAccount(
          const iroha::model::Account &account);
      iroha::protocol::Asset serializeAsset(const iroha::model::Asset &asset);
      iroha::protocol::AccountAsset serializeAccountAsset(
          const iroha::model::AccountAsset &account_asset);
      iroha::protocol::Domain serializeDomain(
          const iroha::model::Domain &domain);
    }  // namespace converters
  }    // namespace model

  /**
   * Calculate hash from protobuf model object
   * @tparam T - protobuf model type
   * @param pb - protobuf model object
   * @return hash of object payload
   */
  template <typename T>
  hash256_t hash(const T &pb) {
    return sha3_256(pb.payload().SerializeAsString());
  }
}  // namespace iroha

#endif  // IROHA_PB_COMMON_HPP
