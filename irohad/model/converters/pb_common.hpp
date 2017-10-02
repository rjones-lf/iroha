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
#include "crypto/crypto.hpp"
#include "crypto/hash.hpp"

namespace iroha {
  namespace model {
    namespace converters {
      // amount
      protocol::Amount serializeAmount(iroha::Amount iroha_amount);
      iroha::Amount deserializeAmount(protocol::Amount pb_amount);
    }
  }

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

  /**
   *
   * @tparam T
   * @param pb
   * @param keypair
   * @return
   */
  template <typename T>
  auto sign(const T &pb, const keypair_t &keypair) {
    return iroha::sign(
        pb.payload().SerializeAsString(), keypair.pubkey, keypair.privkey);
  }

  /**
   *
   * @tparam T
   * @param pb
   * @return
   */
  template <typename T>
  auto verify(const T &pb) {
    return [=](auto signature) {
      return iroha::verify(pb.payload().SerializeAsString(),
                           signature.pubkey,
                           signature.signature);
    };
  }
}

#endif  // IROHA_PB_COMMON_HPP
