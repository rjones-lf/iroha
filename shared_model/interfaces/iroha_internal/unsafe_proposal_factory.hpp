/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_UNSAFE_PROPOSAL_FACTORY_HPP
#define IROHA_UNSAFE_PROPOSAL_FACTORY_HPP

#include <memory>

#include "interfaces/common_objects/types.hpp"

namespace shared_model {
  namespace interface {

    class Proposal;

    /**
     * UnsafeProposalFactory creates proposal without stateless validation
     */
    class UnsafeProposalFactory {
     public:
      virtual std::unique_ptr<Proposal> unsafeCreateProposal(
          types::HeightType height,
          types::TimestampType created_time,
          const types::TransactionsCollectionType &transactions) = 0;

      virtual ~UnsafeProposalFactory() = default;
    };
  }  // namespace interface
}  // namespace shared_model

#endif  // IROHA_UNSAFE_PROPOSAL_FACTORY_HPP
