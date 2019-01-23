/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_MOCK_YAC_SUPERMAJORITY_CHECKER_HPP
#define IROHA_MOCK_YAC_SUPERMAJORITY_CHECKER_HPP

#include <gmock/gmock.h>

#include "consensus/yac/supermajority_checker.hpp"

namespace iroha {
  namespace consensus {
    namespace yac {

      class MockSupermajorityChecker : public SupermajorityChecker {
       public:
        MOCK_CONST_METHOD2(
            hasSupermajority,
            bool(const shared_model::interface::types::SignatureRangeType &,
                 const std::vector<
                     std::shared_ptr<shared_model::interface::Peer>> &));

        MOCK_CONST_METHOD2(checkSize, bool(PeersNumberType, PeersNumberType));

        MOCK_CONST_METHOD2(
            peersSubset,
            bool(const shared_model::interface::types::SignatureRangeType &,
                 const std::vector<
                     std::shared_ptr<shared_model::interface::Peer>> &));

        MOCK_CONST_METHOD3(
            hasReject, bool(PeersNumberType, PeersNumberType, PeersNumberType));
      };

    }  // namespace yac
  }    // namespace consensus
}  // namespace iroha

#endif  // IROHA_MOCK_YAC_SUPERMAJORITY_CHECKER_HPP
