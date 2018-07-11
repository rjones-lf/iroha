/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_TRANSACTION_SEQUENCE_BUILDER_HPP
#define IROHA_TRANSACTION_SEQUENCE_BUILDER_HPP

#include "interfaces/common_objects/types.hpp"
#include "interfaces/iroha_internal/transaction_sequence.hpp"

namespace shared_model {
  namespace proto {

    /**
     * Class for building Transaction Sequence
     * @tparam SV Stateless validator type
     */
    template <typename SV>
    class TransportBuilder<interface::TransactionSequence, SV> {
     private:
      /**
       * Creator of transaction sequence
       * @param transactions collection of transactions
       * @param validator validator of the collections
       * @return Result containing transaction sequence if validation successful
       * and string message containing error otherwise
       */
      template <typename TransactionValidator, typename OrderValidator>
      iroha::expected::Result<interface::TransactionSequence, std::string>
      createTransactionSequence(
          const interface::types::TransactionsForwardCollectionType
              &transactions,
          const validation::TransactionsCollectionValidator<
              TransactionValidator,
              OrderValidator> &validator) {
        auto answer = validator.validate(transactions);
        if (answer.hasErrors()) {
          return iroha::expected::makeError(answer.reason());
        }
        return iroha::expected::makeValue(
            interface::TransactionSequence(transactions));
      }

     public:
      TransportBuilder<interface::TransactionSequence, SV>(
          SV stateless_validator = SV())
          : stateless_validator_(stateless_validator) {}

      /**
       * Builds TransactionSequence from transport object
       * @param transport protobuf object from which TransactionSequence is
       * built
       * @return Result containing either TransactionSequence or message string
       */
      template <class T>
      iroha::expected::Result<interface::TransactionSequence, std::string>
      build(T &transport) {
        return createTransactionSequence(transport, stateless_validator_);
      }

     private:
      SV stateless_validator_;
    };
  }  // namespace proto
}  // namespace shared_model
#endif  // IROHA_TRANSACTION_SEQUENCE_BUILDER_HPP
