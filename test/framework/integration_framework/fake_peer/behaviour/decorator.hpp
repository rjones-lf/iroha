/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef INTEGRATION_FRAMEWORK_FAKE_PEER_BEHAVIOUR_DECORATOR_HPP_
#define INTEGRATION_FRAMEWORK_FAKE_PEER_BEHAVIOUR_DECORATOR_HPP_

#include "framework/integration_framework/fake_peer/behaviour/behaviour.hpp"

namespace integration_framework {
  namespace fake_peer {

    class BehaviourDecorator : public Behaviour {
     public:
      BehaviourDecorator(std::unique_ptr<Behaviour> &&base_behaviour);

      virtual ~BehaviourDecorator() = default;

     protected:
      std::unique_ptr<Behaviour> base_behaviour_;
    };

  }  // namespace fake_peer
}  // namespace integration_framework

#endif /* INTEGRATION_FRAMEWORK_FAKE_PEER_BEHAVIOUR_DECORATOR_HPP_ */
