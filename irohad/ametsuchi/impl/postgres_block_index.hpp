/**
 * Copyright Soramitsu Co., Ltd. 2018 All Rights Reserved.
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

#ifndef IROHA_POSTGRES_BLOCK_INDEX_HPP
#define IROHA_POSTGRES_BLOCK_INDEX_HPP

#include <boost/format.hpp>

#include "ametsuchi/impl/block_index.hpp"
#include "ametsuchi/impl/soci_utils.hpp"
#include "interfaces/transaction.hpp"
#include "logger/logger.hpp"

namespace iroha {
  namespace ametsuchi {
    class PostgresBlockIndex : public BlockIndex {
     public:
      explicit PostgresBlockIndex(soci::session &sql);

      /**
       * Create several indices for block. Namely:
       * transaction hash -> block, where this transaction is stored
       * transaction creator -> block where his transaction is located
       *
       * Additionally, for each Transfer Asset command:
       *   1. (account, asset) -> block for each:
       *     a. creator of the transaction
       *     b. source account
       *     c. destination account
       *   2. account -> block for source and destination accounts
       *   3. (account, height) -> list of txes
       */
      void index(const shared_model::interface::Block &block) override;

     private:
      soci::session &sql_;
      logger::Logger log_;
    };
  }  // namespace ametsuchi
}  // namespace iroha

#endif  // IROHA_POSTGRES_BLOCK_INDEX_HPP
