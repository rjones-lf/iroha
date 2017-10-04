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

#include "model/generators/block_generator.hpp"
#include <chrono>
#include <utility>
#include "crypto/hash.hpp"

namespace iroha {
  namespace model {
    namespace generators {
      Block BlockGenerator::generateGenesisBlock(
          std::vector<std::string> peers_address,
          std::vector<pubkey_t> public_keys) {
        Block block{};
        block.created_ts = 0;
        block.height = 1;
        std::fill(block.prev_hash.begin(), block.prev_hash.end(), 0);
        std::fill(block.merkle_root.begin(), block.merkle_root.end(), 0);
        block.txs_number = 1;
        TransactionGenerator tx_generator;
        block.transactions = {
            tx_generator.generateGenesisTransaction(block.created_ts,
                                                    std::move(peers_address),
                                                    std::move(public_keys))};
        block.hash = hash(block);

        return block;
      }

      Block BlockGenerator::generateGenesisBlock(
          std::vector<std::string> peers_address) {
        Block block{};
        block.created_ts = 0;
        block.height = 1;
        std::fill(block.prev_hash.begin(), block.prev_hash.end(), 0);
        std::fill(block.merkle_root.begin(), block.merkle_root.end(), 0);
        block.txs_number = 1;
        TransactionGenerator tx_generator;
        block.transactions = {tx_generator.generateGenesisTransaction(
            block.created_ts, std::move(peers_address))};
        block.hash = hash(block);

        return block;
      }

    }  // namespace generators
  }    // namespace model
}  // namespace iroha
