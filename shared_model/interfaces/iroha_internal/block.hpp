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
#include "interfaces/common_objects/types.hpp"
#include "interfaces/transaction.hpp"
#include "utils/string_builder.hpp"

namespace shared_model {
  namespace interface {

    class Block : public Signable<Block> {
     public:
      /**
       * @return block number in the ledger
       */
      virtual types::HeightType height() const = 0;

      /**
       * @return hash of a previous block
       */
      virtual const types::HashType &prevHash() const = 0;

      /**
       * @return amount of transactions in block
       */
      virtual types::TransactionsNumberType txsNumber() const = 0;

      /**
       * @return collection of transactions
       */
      virtual const types::TransactionsCollectionType &transactions() const = 0;

      std::string toString() const override {
        return detail::PrettyStringBuilder()
            .init("Block")
            .append("hash", hash().hex())
            .append("height", std::to_string(height()))
            .append("prevHash", prevHash().hex())
            .append("txsNumber", std::to_string(txsNumber()))
            .append("createdtime", std::to_string(createdTime()))
            .append("transactions")
            .appendAll(transactions(), [](auto &tx) { return tx->toString(); })
            .append("signatures")
            .appendAll(signatures(), [](auto &sig) { return sig.toString(); })
            .finalize();
      }
    };

  }  // namespace interface
}  // namespace shared_model
#endif  // IROHA_SHARED_MODEL_BLOCK_HPP
