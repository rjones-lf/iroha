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

#ifndef IROHA_BLOCK_LOADER_HPP
#define IROHA_BLOCK_LOADER_HPP

#include <rxcpp/rx-observable.hpp>

#include "model/block.hpp"
#include "model/peer.hpp"

namespace iroha {
  namespace network {

    /**
     * Interface for downloading blocks from a network
     */
    class BlockLoader {
     public:

      /**
       * Retrieve block from given peer starting from current top
       * @param peer_pubkey - peer for requesting blocks
       * @return
       */
      virtual rxcpp::observable<model::Block> retrieveBlocks(
          model::Peer::KeyType peer_pubkey) = 0;

      /**
       * Retrieve block by its block_hash from given peer
       * @param peer_pubkey - peer for requesting blocks
       * @param block_hash - requested block hash
       * @return block on success, nullopt on failure
       */
      virtual nonstd::optional<model::Block> retrieveBlock(
          model::Peer::KeyType peer_pubkey,
          model::Block::HashType block_hash) = 0;

      virtual ~BlockLoader() = default;
    };
  } // namespace network
} // namespace iroha

#endif //IROHA_BLOCK_LOADER_HPP
