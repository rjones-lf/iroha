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

#ifndef IROHA_BLOCK_INDEX_HPP
#define IROHA_BLOCK_INDEX_HPP

#include <memory>

namespace shared_model {
  namespace interface {
    class Block;
  }  // namespace interface
}  // namespace shared_model

namespace iroha {
  namespace ametsuchi {
    /**
     * Internal interface for modifying index on blocks and transactions
     */
    class BlockIndex {
     public:
      virtual ~BlockIndex() = default;

      /**
       * Add block to index
       * @param block to be indexed
       */
      virtual void index(const shared_model::interface::Block &) = 0;
    };
  }  // namespace ametsuchi
}  // namespace iroha

#endif  // IROHA_BLOCK_INDEX_HPP
