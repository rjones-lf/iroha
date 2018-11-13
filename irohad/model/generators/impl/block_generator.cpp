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

#include "model/sha3_hash.hpp"

namespace iroha {
  namespace model {
    namespace generators {
      Block BlockGenerator::generateGenesisBlock(
          ts64_t created_ts, const std::vector<Transaction> &transactions) {
        Block block{};
        block.created_ts = created_ts;
        block.height = 1;
        std::fill(block.prev_hash.begin(), block.prev_hash.end(), 0);
        block.txs_number = 1;
        block.transactions = transactions;
        block.hash = hash(block);

        return block;
      }

    }  // namespace generators
  }    // namespace model
}  // namespace iroha
