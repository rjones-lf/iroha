/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "framework/integration_framework/fake_peer/behaviour/honest.hpp"

namespace integration_framework {
  namespace fake_peer {

    void HonestBehaviour::processYacMessage(FakePeer::YacMessagePtr message) {
      getFakePeer().voteForTheSame(message);
    }

    std::string HonestBehaviour::getName() {
      return "honest behaviour";
    }

  }  // namespace fake_peer
}  // namespace integration_framework
