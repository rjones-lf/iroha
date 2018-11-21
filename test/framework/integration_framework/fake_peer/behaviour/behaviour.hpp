/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef INTEGRATION_FRAMEWORK_FAKE_PEER_BEHAVIOUR_HPP_
#define INTEGRATION_FRAMEWORK_FAKE_PEER_BEHAVIOUR_HPP_

#include <memory>
#include <vector>

#include "framework/integration_framework/fake_peer/fake_peer.hpp"
#include "logger/logger.hpp"

namespace integration_framework {
  namespace fake_peer {

    class Behaviour {
     public:
      virtual ~Behaviour();

      /// Enable the behaviour for the given peer
      void adopt(const std::shared_ptr<FakePeer> &fake_peer);

      /// Disable the behaviour
      void absolve();

      /// This method gets subscribed on Fake Peer's MST messages.
      virtual void processMstMessage(FakePeer::MstMessagePtr message) = 0;

      /// This method gets subscribed on Fake Peer's YAC messages.
      virtual void processYacMessage(FakePeer::YacMessagePtr message) = 0;

      /// This method gets subscribed on Fake Peer's OS messages.
      virtual void processOsBatch(FakePeer::OsBatchPtr batch) = 0;

      /// This method gets subscribed on Fake Peer's OG messages.
      virtual void processOgProposal(FakePeer::OgProposalPtr proposal) = 0;

      virtual std::string getName() = 0;

     protected:
      FakePeer &getFakePeer();

     private:
      std::weak_ptr<FakePeer> fake_peer_wptr_;
      std::vector<rxcpp::subscription> subscriptions_;
      logger::Logger log_;
    };

  }  // namespace fake_peer
}  // namespace integration_framework

#endif /* INTEGRATION_FRAMEWORK_FAKE_PEER_BEHAVIOUR_HPP_ */
