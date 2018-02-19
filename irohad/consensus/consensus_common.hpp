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

#ifndef IROHA_CONSENSUS_COMMON_HPP
#define IROHA_CONSENSUS_COMMON_HPP

#include <algorithm>
#include <cstdint>
#include <vector>

#include "model/peer.hpp"
#include "model/signature.hpp"
#include "interfaces/iroha_internal/block.hpp"

namespace iroha {
  namespace consensus {
    /**
     * Check that current number >= supermajority.
     * @param current - current number for validation
     * @param all - whole number (N)
     * @return true if belong supermajority
     */
    inline bool hasSupermajority(uint64_t current, uint64_t all) {
      if (current > all) {
        return false;
      }
      auto f = (all - 1) / 3.0;
      return current >= 2 * f + 1;
    }

    /**
     * Checks if public keys of signatures are present in peers collection
     * @param signatures - collection of signatures
     * @param peers - collection of peers
     * @return true, if all public keys of signatures are present in peers
     * collection, false otherwise
     */
    inline bool peersSubset(const shared_model::interface::SignatureSetType &signatures,
                            std::vector<model::Peer> peers) {
      return std::all_of(
          signatures.begin(), signatures.end(), [peers](auto signature) {
            return std::find_if(peers.begin(),
                                peers.end(),
                                [signature](auto peer) {

                                  // TODO: 14-02-2018 Alexey Chernyshov remove this after relocation to
                                  // shared_model https://soramitsu.atlassian.net/browse/IR-903
                                  shared_model::crypto::PublicKey new_pubkey(
                                  {peer.pubkey.begin(), peer.pubkey.end()});

                                  return signature->publicKey() == new_pubkey;
                                })
                != peers.end();
          });
    }
  }  // namespace consensus
}  // namespace iroha

#endif  // IROHA_CONSENSUS_COMMON_HPP
