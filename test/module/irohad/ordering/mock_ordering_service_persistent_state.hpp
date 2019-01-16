/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef IROHA_MOCK_ORDERING_SERVICE_PERSISTENT_STATE_HPP
#define IROHA_MOCK_ORDERING_SERVICE_PERSISTENT_STATE_HPP

#include <gmock/gmock.h>

#include "ametsuchi/ordering_service_persistent_state.hpp"

class MockOrderingServicePersistentState
    : public iroha::ametsuchi::OrderingServicePersistentState {
 public:
  /**
   * Save proposal height
   */
  MOCK_METHOD1(saveProposalHeight, bool(size_t height));

  /**
   * Load proposal height
   */
  MOCK_CONST_METHOD0(loadProposalHeight, boost::optional<size_t>());

  /**
   * Reset state
   */
  MOCK_METHOD0(resetState, bool());
};

#endif  // IROHA_MOCK_ORDERING_SERVICE_PERSISTENT_STATE_HPP
