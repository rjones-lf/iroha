#include "framework/integration_framework/fake_peer/behaviour/decorator.hpp"

namespace integration_framework {
  namespace fake_peer {

    BehaviourDecorator::BehaviourDecorator(
        std::unique_ptr<Behaviour> &&base_behaviour)
        : base_behaviour_(std::move(base_behaviour)) {}

  }  // namespace fake_peer
}  // namespace integration_framework
