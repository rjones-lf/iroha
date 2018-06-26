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

    /// Type of per-transaction errors, which appeared during validation
    /// process; contains errors themselves and transaction hashes
    using TransactionsErrors = std::vector<
        std::pair<std::string, shared_model::interface::types::HashType>>;

    /// Type of verified proposal and errors appeared in the process; first
    /// dimension of errors vector is transaction, second is error itself with
    /// number of transaction, where it happened
    using VerifiedProposalAndErrors =
        std::pair<std::shared_ptr<shared_model::interface::Proposal>,
                  TransactionsErrors>;

  }  // namespace validation
}  // namespace iroha

#endif  // IROHA_STATEFUL_VALIDATOR_COMMON_HPP
