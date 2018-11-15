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

#ifndef IROHA_BLOCK_HPP
#define IROHA_BLOCK_HPP

#include <vector>

#include "crypto/hash_types.hpp"
#include "model/proposal.hpp"
#include "model/signature.hpp"
#include "model/transaction.hpp"

namespace iroha {
  namespace model {

    /**
     * Block is Model-structure, that provides all block-related information
     * Block can be divided into payload which contains all the data
     * and signatures of this data.
     */
    struct Block {
      /**
       * Calculated as hash(PAYLOAD field)
       * NOT a part of payload
       */
      hash256_t hash{};

      using HashType = decltype(hash);

      /**
       * List of signatures for signing the block
       * NOT a part of PAYLOAD
       */
      std::vector<Signature> sigs;

      using SignaturesType = decltype(sigs);

      /**
       * Timestamp of block creation(signing)
       * part of PAYLOAD
       */
      ts64_t created_ts{};

      /**
       * Block number in the ledger
       * Height can be used as block_id
       * part of PAYLOAD
       */
      uint64_t height{};

      using BlockHeightType = decltype(height);

      /**
       * Hash of a previous block in the ledger
       * part of PAYLOAD
       */
      hash256_t prev_hash{};

      /**
       * Number of transactions in block body
       * part of PAYLOAD
       */
      uint16_t txs_number{};

      /**
       * Attached transactions
       * part of PAYLOAD
       */
      std::vector<Transaction> transactions;

      using TransactionsType = decltype(transactions);

      /**
       * Attached rejected transactions' hashes
       * part of PAYLOAD
       */
      std::vector<HashType> rejected_transactions_hashes;

      using RejectedTransactionsType = decltype(rejected_transactions_hashes);

      bool operator==(const Block &rhs) const;
      bool operator!=(const Block &rhs) const;
    };
  }  // namespace model
}  // namespace iroha

#endif  // IROHA_BLOCK_HPP
