/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "framework/integration_framework/fake_peer/behaviour/honest.hpp"

#include "backend/protobuf/block.hpp"
#include "framework/integration_framework/fake_peer/block_storage.hpp"

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

  }  // namespace fake_peer
}  // namespace integration_framework
