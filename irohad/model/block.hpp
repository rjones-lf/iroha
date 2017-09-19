/*
Copyright Soramitsu Co., Ltd. 2016 All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#ifndef IROHA_BLOCK_HPP
#define IROHA_BLOCK_HPP

#include <common/types.hpp>
#include <model/signature.hpp>
#include <model/transaction.hpp>
#include <vector>
#include <model/proposal.hpp>

namespace iroha {
  namespace model {

    /**
     * Block is Model-structure,  that provides all block-related information
     * Block can be divided into three abstractions: {Header, Meta, Body}.
     *
     */
    struct Block {
      /**
       * Calculated as sha3_256(META + BODY fields)
       * HEADER field
       */
      hash256_t hash;

      using HashType = decltype(hash);

      /**
       * List of signatures for signing the block
       * HEADER field
       */
      std::vector<Signature> sigs;

      using SignaturesType = decltype(sigs);

      /**
       * Timestamp of block creation(signing)
       * HEADER field
       */
      ts64_t created_ts;

      /**
       * Block number in the ledger
       * Height can be used as block_id
       * META field
       */
      uint64_t height;

      /**
       * Hash of a previous block in the ledger
       * META field
       */
      hash256_t prev_hash;

      /**
       * Number of transactions in block body
       * META field
       */
      uint16_t txs_number;

      /**
       * Root of merkle tree based on the block and all previous blocks
       * in the ledger
       * META field
       */
      hash256_t merkle_root;

      /**
       * Attached transactions
       * BODY field
       */
      std::vector<Transaction> transactions;

      using TransactionsType = decltype(transactions);

      bool operator==(const Block& rhs) const;
      bool operator!=(const Block& rhs) const;
    };
  }
}

#endif  // IROHA_BLOCK_HPP
