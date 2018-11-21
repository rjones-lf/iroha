/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef INTEGRATION_FRAMEWORK_FAKE_PEER_BEHAVIOUR_EMPTY_HPP_
#define INTEGRATION_FRAMEWORK_FAKE_PEER_BEHAVIOUR_EMPTY_HPP_

#include <memory>
#include <vector>

#include "framework/integration_framework/fake_peer/behaviour/behaviour.hpp"
#include "framework/integration_framework/fake_peer/fake_peer.hpp"
#include "logger/logger.hpp"

namespace integration_framework {
  namespace fake_peer {

    class EmptyBehaviour : public Behaviour {
     public:
      virtual ~EmptyBehaviour() = default;

      void processMstMessage(FakePeer::MstMessagePtr message) override;
      void processYacMessage(FakePeer::YacMessagePtr message) override;
      void processOsBatch(FakePeer::OsBatchPtr batch) override;
      void processOgProposal(FakePeer::OgProposalPtr proposal) override;

      virtual std::string getName();
    };

  }  // namespace fake_peer
}  // namespace integration_framework

#endif /* INTEGRATION_FRAMEWORK_FAKE_PEER_BEHAVIOUR_EMPTY_HPP_ */
