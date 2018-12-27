/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_ORDERING_SERVICE_PERSISTENT_STATE_HPP
#define IROHA_ORDERING_SERVICE_PERSISTENT_STATE_HPP

#include <boost/optional.hpp>

namespace iroha {
  namespace ametsuchi {

    /**
     * Interface for Ordering Service persistence to store proposal's height in
     * a persistent way
     */
    class OrderingServicePersistentState {
     public:
      /**
       * Save proposal height that it can be restored
       * after launch
       */
      virtual bool saveProposalHeight(size_t height) = 0;

      /**
       * Load proposal height
       */
      virtual boost::optional<size_t> loadProposalHeight() const = 0;

      /**
       * Reset storage to default state
       */
      virtual bool resetState() = 0;

      virtual ~OrderingServicePersistentState() = default;
    };
  }  // namespace ametsuchi
}  // namespace iroha

#endif  // IROHA_ORDERING_SERVICE_PERSISTENT_STATE_HPP
