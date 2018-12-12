/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_SUPERMAJORITY_CHECKER_CFT_HPP
#define IROHA_SUPERMAJORITY_CHECKER_CFT_HPP

#include "consensus/yac/supermajority_checker.hpp"

namespace iroha {
  namespace consensus {
    namespace yac {

      namespace detail {
        constexpr unsigned int kSupermajorityCheckerKfPlus1Cft = 2;
      }

      /// An implementation of CFT supermajority checker.
      class SupermajorityCheckerCft : public SupermajorityChecker {
       public:
        virtual ~SupermajorityCheckerCft() = default;

        bool hasSupermajority(PeersNumberType current,
                              PeersNumberType all) const override;

        bool canHaveSupermajority(const VoteGroups &votes,
                                  PeersNumberType all) const override;
      };
    }  // namespace yac
  }    // namespace consensus
}  // namespace iroha

#endif  // IROHA_SUPERMAJORITY_CHECKER_CFT_HPP
