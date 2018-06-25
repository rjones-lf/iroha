/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_STATEFUL_VALIDATOR_COMMON_HPP
#define IROHA_STATEFUL_VALIDATOR_COMMON_HPP

namespace iroha {
  namespace validation {

    /// Type of verified proposal and errors appeared in the process; first
    /// dimension of errors vector is transaction, second is error itself with
    /// number of transaction, where it happened
    using VerifiedProposalAndErrors =
        std::pair<std::shared_ptr<shared_model::interface::Proposal>,
                  std::vector<std::pair<std::string, size_t>>>;

  }  // namespace validation
}  // namespace iroha

#endif  // IROHA_STATEFUL_VALIDATOR_COMMON_HPP
