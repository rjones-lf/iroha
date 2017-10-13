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

#ifndef IROHA_HASH_HPP
#define IROHA_HASH_HPP

#include <functional>
#include <string>
#include "multi_sig_transactions/mst_types.hpp"

namespace iroha {
  namespace model {
    /**
     * Hash calculation factory for transaction
     */
    template <typename Tx>
    class PointerTxHasher {
     public:
      using TxType = Tx;
      size_t operator()(const TxType &tx) const {
        auto hash = string_hasher(tx->tx_hash.to_string());
        return hash;
      }

     private:
      std::hash<std::string> string_hasher;
    };

    /**
     * Hash calculation factory for signature
     */
    class SignatureHasher {
     public:
      size_t operator()(const iroha::model::Signature &sign) const {
        auto hash =
            string_hasher(sign.signature.to_string() + sign.pubkey.to_string());
        return hash;
      }

     private:
      std::hash<std::string> string_hasher;
    };

    /**
     * Hasing of peer object
     */
    class PeerHasher {
     public:
      std::size_t operator()(ConstPeer &obj) const {
        return hasher(obj.address + obj.pubkey.to_string());
      }

     private:
      std::hash<std::string> hasher;
    };
  }  // namespace model
}  // namespace iroha

#endif  // IROHA_HASH_HPP
