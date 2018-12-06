/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "main/impl/block_loader_init.hpp"
#include "validators/default_validator.hpp"

using namespace iroha;
using namespace iroha::ametsuchi;
using namespace iroha::network;

auto BlockLoaderInit::createService(
    std::shared_ptr<BlockQueryFactory> block_query_factory,
    std::shared_ptr<consensus::ConsensusResultCache> consensus_result_cache) {
  return std::make_shared<BlockLoaderService>(
      std::move(block_query_factory), std::move(consensus_result_cache));
}

auto BlockLoaderInit::createLoader(
    std::shared_ptr<PeerQueryFactory> peer_query_factory) {
  shared_model::proto::ProtoBlockFactory factory(
      std::make_unique<
          shared_model::validation::DefaultSignedBlockValidator>());
  return std::make_shared<BlockLoaderImpl>(std::move(peer_query_factory),
                                           std::move(factory));
}

std::shared_ptr<BlockLoader> BlockLoaderInit::initBlockLoader(
    std::shared_ptr<PeerQueryFactory> peer_query_factory,
    std::shared_ptr<BlockQueryFactory> block_query_factory,
    std::shared_ptr<consensus::ConsensusResultCache> consensus_result_cache) {
  service = createService(std::move(block_query_factory),
                          std::move(consensus_result_cache));
  loader = createLoader(std::move(peer_query_factory));
  return loader;
}
