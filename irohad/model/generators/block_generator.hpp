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

#include "model/block.hpp"
#include "model/generators/transaction_generator.hpp"

#ifndef IROHA_BLOCK_GENERATOR_HPP
#define IROHA_BLOCK_GENERATOR_HPP
namespace iroha {
  namespace model {
    namespace generators {

      class BlockGenerator {
       public:
        /**
         * Generate sample genesis for new Iroha network
         * @param peers_address
         * @param transactions
         * @return model Block
         */
        iroha::model::Block generateGenesisBlock(
          ts64_t created_ts,
          const std::vector<iroha::model::Transaction>& transactions);

        /**
         * Generate block from give meta data and transaction list
         * @param created_ts
         * @param height
         * @param prev_hash
         * @param transactions
         * @return
         */
        iroha::model::Block generateBlock(
          ts64_t created_ts,
          uint64_t height,
          const iroha::hash256_t &prev_hash,
          const std::vector<iroha::model::Transaction> &transactions);
      };

    }  // namespace generators
  }    // namespace model
}  // namespace iroha

#endif  // IROHA_BLOCK_GENERATOR_HPP
