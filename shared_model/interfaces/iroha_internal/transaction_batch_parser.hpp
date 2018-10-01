/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_TRANSACTION_BATCH_PARSER_HPP
#define IROHA_TRANSACTION_BATCH_PARSER_HPP

#include "interfaces/common_objects/transaction_sequence_common.hpp"
#include "interfaces/common_objects/types.hpp"

namespace shared_model {
  namespace interface {

    /**
     * Parses a transaction list and return a list of possible batches
     */
    class TransactionBatchParser {
     public:
      virtual std::vector<types::TransactionsForwardCollectionType>
      parseBatches(types::TransactionsForwardCollectionType txs) = 0;

      virtual std::vector<types::TransactionsCollectionType> parseBatches(
          types::TransactionsCollectionType txs) = 0;

      virtual std::vector<types::SharedTxsCollectionType> parseBatches(
          const types::SharedTxsCollectionType &txs) = 0;

      virtual ~TransactionBatchParser() = default;
    };
  }  // namespace interface
}  // namespace shared_model

#endif  // IROHA_TRANSACTION_BATCH_PARSER_HPP
