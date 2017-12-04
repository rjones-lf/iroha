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

#ifndef IROHA_SHARED_MODEL_BLOCK_HPP
#define IROHA_SHARED_MODEL_BLOCK_HPP

#include "interfaces/base/signable.hpp"
#include "interfaces/transaction.hpp"
#include "model/block.hpp"
#include "utils/string_builder.hpp"

namespace shared_model {
  namespace interface {

    class Block : public Signable<Block, iroha::model::Block> {
     public:
      /**
       * @return block number in the ledger
       */
      virtual types::HeightType &height() const = 0;

      /**
       * @return hash of a previous block
       */
      virtual HashType &prevHash() const = 0;

      /// Type of a number of transactions in block
      using TransactionsNumberType = uint16_t;

      /**
       * @return amount of transactions in block
       */
      virtual TransactionsNumberType &txsNumber() const = 0;

      /**
       * @return Root of merkle tree based on the block and all previous blocks
       * in the ledger
       */
      virtual HashType &merkleRoot() const = 0;

      /// Type of a single Transaction
      using TransactionType = detail::PolymorphicWrapper<Transaction>;

      /// Type of transactions' collection
      using TransactionsCollectionType = std::vector<TransactionType>;

      /**
       * @return collection of transactions
       */
      virtual const TransactionsCollectionType &transactions() const = 0;

      iroha::model::Block *makeOldModel() const override {
        iroha::model::Block *oldStyleBlock = new iroha::model::Block();
        oldStyleBlock->height = height();
        oldStyleBlock->prev_hash =
            iroha::model::Block::HashType::from_string(prevHash().toString());
        oldStyleBlock->txs_number = txsNumber();
        oldStyleBlock->merkle_root =
            iroha::model::Block::HashType::from_string(merkleRoot().toString());
        std::for_each(
            transactions().begin(),
            transactions().end(),
            [oldStyleBlock](auto &tx) {
              oldStyleBlock->transactions.emplace_back(*tx->makeOldModel());
            });
        oldStyleBlock->created_ts = createdTime();
        oldStyleBlock->hash =
            iroha::model::Block::HashType::from_string(hash().toString());
        std::for_each(signatures().begin(),
                      signatures().end(),
                      [oldStyleBlock](auto &sig) {
                        oldStyleBlock->sigs.emplace_back(*sig->makeOldModel());
                      });
        return oldStyleBlock;
      }

      std::string toString() const override {
        return detail::PrettyStringBuilder()
            .init("Block")
            .append("hash", hash().hex())
            .append("height", std::to_string(height()))
            .append("prevHash", prevHash().hex())
            .append("txsNumber", std::to_string(txsNumber()))
            .append("merkleRoot", merkleRoot().hex())
            .append("createdtime", std::to_string(createdTime()))
            .append("transactions")
            .appendAll(transactions(), [](auto &tx) { return tx->toString(); })
            .append("signatures")
            .appendAll(signatures(), [](auto &sig) { return sig->toString(); })
            .finalize();
      }
    };

  }  // namespace interface
}  // namespace shared_model
#endif  // IROHA_SHARED_MODEL_BLOCK_HPP
