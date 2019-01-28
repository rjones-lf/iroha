/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "framework/integration_framework/fake_peer/behaviour/empty.hpp"

namespace integration_framework {
  namespace fake_peer {

    void EmptyBehaviour::processMstMessage(MstMessageSPtr message) {}
    void EmptyBehaviour::processYacMessage(YacMessageSPtr message) {}
    void EmptyBehaviour::processOsBatch(OsBatchSPtr batch) {}
    void EmptyBehaviour::processOgProposal(OgProposalSPtr proposal) {}
    LoaderBlockRequestResult EmptyBehaviour::processLoaderBlockRequest(
        LoaderBlockRequest request) {
      return {};
    }
    LoaderBlocksRequestResult EmptyBehaviour::processLoaderBlocksRequest(
        LoaderBlocksRequest request) {
      return {};
    }
    OrderingProposalRequestResult
    EmptyBehaviour::processOrderingProposalRequest(
        const OrderingProposalRequest &request) {
      return {};
    }
    void EmptyBehaviour::processOrderingBatches(
        const BatchesForRound &batches_for_round) {}

    std::string EmptyBehaviour::getName() {
      return "empty behaviour";
    }

  }  // namespace fake_peer
}  // namespace integration_framework
