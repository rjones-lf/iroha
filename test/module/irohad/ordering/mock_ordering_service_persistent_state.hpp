/**
 * Copyright Soramitsu Co., Ltd. 2018 All Rights Reserved.
 * http://soramitsu.co.jp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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
