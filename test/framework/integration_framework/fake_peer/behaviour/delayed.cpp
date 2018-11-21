#include "framework/integration_framework/fake_peer/behaviour/delayed.hpp"

namespace integration_framework {
  namespace fake_peer {

    DelayedBehaviour::DelayedBehaviour(
        std::unique_ptr<Behaviour> &&base_behaviour,
        std::chrono::milliseconds delay)
        : BehaviourDecorator(std::move(base_behaviour)), delay_(delay) {}

    void DelayedBehaviour::processMstMessage(MstMessagePtr message) {
      std::this_thread::sleep_for(delay_);
      base_behaviour_->processMstMessage(message);
    }

    void DelayedBehaviour::processYacMessage(YacMessagePtr message) {
      std::this_thread::sleep_for(delay_);
      base_behaviour_->processYacMessage(message);
    }

    void DelayedBehaviour::processOsBatch(OsBatchPtr batch) {
      std::this_thread::sleep_for(delay_);
      base_behaviour_->processOsBatch(batch);
    }

    void DelayedBehaviour::processOgProposal(OgProposalPtr proposal) {
      std::this_thread::sleep_for(delay_);
      base_behaviour_->processOgProposal(proposal);
    }

    LoaderBlockRequestResult DelayedBehaviour::processLoaderBlockRequest(
        LoaderBlockRequest request) {
      std::this_thread::sleep_for(delay_);
      return base_behaviour_->processLoaderBlockRequest(request);
    }

    LoaderBlocksRequestResult DelayedBehaviour::processLoaderBlocksRequest(
        LoaderBlocksRequest request) {
      std::this_thread::sleep_for(delay_);
      return base_behaviour_->processLoaderBlocksRequest(request);
    }

    std::string DelayedBehaviour::getName() {
      return "delayed " + base_behaviour_->getName();
    }

  }  // namespace fake_peer
}  // namespace integration_framework
