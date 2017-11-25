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
          ts64_t created_ts,
          const std::vector<Transaction>& transactions) {
        Block block{};
        block.created_ts = created_ts;
        block.height = 1;
        std::fill(block.prev_hash.begin(), block.prev_hash.end(), 0);
        std::fill(block.merkle_root.begin(), block.merkle_root.end(), 0);
        block.txs_number = 1;
        block.transactions = transactions;
        block.hash = hash(block);

        return block;
      }

      iroha::model::Block BlockGenerator::generateBlock(
          ts64_t created_ts,
          uint64_t height,
          const iroha::hash256_t &prev_hash,
          const std::vector<iroha::model::Transaction> &transactions) {
        iroha::model::Block block;
        block.created_ts = created_ts;
        block.transactions = transactions;
        block.height = height;
        block.txs_number = static_cast<uint16_t>(block.transactions.size());
        block.prev_hash = prev_hash;
        block.hash = iroha::hash(block);
        return block;
      }
    }  // namespace generators
  }    // namespace model
}  // namespace iroha
