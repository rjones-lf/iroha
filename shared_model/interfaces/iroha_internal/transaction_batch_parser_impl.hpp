/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_TRANSACTION_BATCH_PARSER_IMPL_HPP
#define IROHA_TRANSACTION_BATCH_PARSER_IMPL_HPP

#include "interfaces/iroha_internal/transaction_batch_parser.hpp"

namespace shared_model {
  namespace interface {

    class TransactionBatchParserImpl : public TransactionBatchParser {
     public:
      std::vector<types::TransactionsForwardCollectionType> parseBatches(
          types::TransactionsForwardCollectionType txs) override;

      std::vector<types::TransactionsCollectionType> parseBatches(
          types::TransactionsCollectionType txs) override;

      std::vector<types::SharedTxsCollectionType> parseBatches(
          const types::SharedTxsCollectionType &txs) override;
    };
  }  // namespace interface
}  // namespace shared_model

#endif  // IROHA_TRANSACTION_BATCH_PARSER_IMPL_HPP
