/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_TRANSACTION_SEQUENCE_HPP
#define IROHA_TRANSACTION_SEQUENCE_HPP

#include "common/result.hpp"
#include "interfaces/common_objects/types.hpp"

namespace shared_model {
  namespace interface {

    namespace types {
      using TransactionForwardCollectionType =
          boost::any_range<Transaction,
                           boost::forward_traversal_tag,
                           const Transaction &>;
    }

    /**
     * Transaction sequence is the collection of transactions where:
     * 1. All transactions from the same batch are place contiguously
     * 2. All batches are full (no transaction from the batch can be outside
     * sequence)
     */
    class TransactionSequence {
     public:
      /**
       * Creator of transaction sequence
       * @tparam Validator checks validation logic
       * @param transactions collection of transactions
       * @return valid transaction sequence
       */
      template <typename Validator>
      iroha::expected::Result<TransactionSequence, std::string>
      createTransactionSequence(
          const types::TransactionForwardCollectionType &transactions);

      /**
       * Get transactions collection
       * @return transactions collection
       */
      types::TransactionForwardCollectionType transactions();

     private:
      explicit TransactionSequence(
          const types::TransactionForwardCollectionType &transactions);

      types::TransactionForwardCollectionType transactions_;
    };

  }  // namespace interface
}  // namespace shared_model

#endif  // IROHA_TRANSACTION_SEQUENCE_HPP
