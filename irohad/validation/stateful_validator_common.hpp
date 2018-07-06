/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_STATEFUL_VALIDATOR_COMMON_HPP
#define IROHA_STATEFUL_VALIDATOR_COMMON_HPP

#include <vector>

#include "interfaces/common_objects/types.hpp"

namespace shared_model {
  namespace interface {
    class Proposal;
  }
}  // namespace shared_model

namespace iroha {
  namespace validation {

    /// Type of command error report
    struct CommandError {
      /// Name of the failed command
      std::string name;

      /// Error, with which the command failed
      std::string error;

      /// Shows, if transaction has passed initial validation
      bool tx_passed_initial_validation;

      /// Position of the failed command in transaction
      size_t index = 0;
    };

    /// Type of transaction error, which appeared during validation
    /// process; contains names of commands, commands errors themselves,
    /// commands indices and transaction hashes
    using TransactionError =
        std::pair<CommandError, shared_model::interface::types::HashType>;

    /// Collection of transactions errors
    using TransactionsErrors = std::vector<TransactionError>;

    /// Type of verified proposal and errors appeared in the process; first
    /// dimension of errors vector is transaction, second is error itself with
    /// number of transaction, where it happened
    using VerifiedProposalAndErrors =
        std::pair<std::shared_ptr<shared_model::interface::Proposal>,
                  TransactionsErrors>;

  }  // namespace validation
}  // namespace iroha

#endif  // IROHA_STATEFUL_VALIDATOR_COMMON_HPP
