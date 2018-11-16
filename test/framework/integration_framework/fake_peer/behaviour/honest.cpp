/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "framework/integration_framework/fake_peer/behaviour/honest.hpp"

#include "backend/protobuf/block.hpp"
#include "framework/integration_framework/fake_peer/block_storage.hpp"
#include "framework/integration_framework/fake_peer/proposal_storage.hpp"

namespace integration_framework {
  namespace fake_peer {

    void HonestBehaviour::processYacMessage(YacMessagePtr message) {
      getFakePeer().voteForTheSame(message);
    }

    std::string HonestBehaviour::getName() {
      return "honest behaviour";
    }

    LoaderBlockRequestResult HonestBehaviour::processLoaderBlockRequest(
        LoaderBlockRequest request) {
      const auto block_storage = getFakePeer().getBlockStorage();
      if (!block_storage) {
        getLogger()->debug(
            "Got a Loader.retrieveBlock call, but have no block storage!");
        return {};
      }
      const auto block = block_storage->getBlockByHash(*request);
      if (!block) {
        getLogger()->debug(
            "Got a Loader.retrieveBlock call for {}, but have no such block!",
            request->toString());
        return {};
      }
      return *std::static_pointer_cast<shared_model::proto::Block>(block);
    }

    LoaderBlocksRequestResult HonestBehaviour::processLoaderBlocksRequest(
        LoaderBlocksRequest request) {
      const auto block_storage = getFakePeer().getBlockStorage();
      if (!block_storage) {
        getLogger()->debug(
            "Got a Loader.retrieveBlocks call, but have no block storage!");
        return {};
      }
      BlockStorage::HeightType current_height = request;
      BlockStorage::BlockPtr block;
      LoaderBlocksRequestResult blocks;
      while ((block = block_storage->getBlockByHeight(current_height++))
             != nullptr) {
        blocks.emplace_back(*block);
      }
      return blocks;
    }

    OrderingProposalRequestResult
    HonestBehaviour::processOrderingProposalRequest(
        const OrderingProposalRequest &request) {
      const auto proposal_storage = getFakePeer().getProposalStorage();
      if (!proposal_storage) {
        getLogger()->debug(
            "Got an OnDemandOrderingService.GetProposal call for round {}, "
            "but have no proposal storage! NOT returning a proposal.",
            request.toString());
        return boost::none;
      }
      return proposal_storage->getProposal(request);
    }

  }  // namespace fake_peer
}  // namespace integration_framework
