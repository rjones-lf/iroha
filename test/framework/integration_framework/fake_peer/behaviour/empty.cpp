#include "framework/integration_framework/fake_peer/behaviour/empty.hpp"

namespace integration_framework {
  namespace fake_peer {

    void EmptyBehaviour::processMstMessage(FakePeer::MstMessagePtr message) {}
    void EmptyBehaviour::processYacMessage(FakePeer::YacMessagePtr message) {}
    void EmptyBehaviour::processOsBatch(FakePeer::OsBatchPtr batch) {}
    void EmptyBehaviour::processOgProposal(FakePeer::OgProposalPtr proposal) {}

    std::string EmptyBehaviour::getName() {
      return "empty behaviour";
    }

  }  // namespace fake_peer
}  // namespace integration_framework
