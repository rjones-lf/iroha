/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_BLOCK_LOADER_INIT_HPP
#define IROHA_BLOCK_LOADER_INIT_HPP

#include "ametsuchi/block_query_factory.hpp"
#include "consensus/consensus_block_cache.hpp"
#include "network/impl/block_loader_impl.hpp"
#include "network/impl/block_loader_service.hpp"

namespace iroha {
  namespace network {
    /**
     * Initialization context of Block loader: loader itself and service
     */
    class BlockLoaderInit {
     private:
      /**
       * Create block loader service with given storage
       * @param block_query_factory - factory to block query component
       * @param block_cache used to retrieve last block put by consensus
       * @return initialized service
       */
      auto createService(
          std::shared_ptr<ametsuchi::BlockQueryFactory> block_query_factory,
          std::shared_ptr<consensus::ConsensusResultCache> block_cache);

      /**
       * Create block loader for loading blocks from given peer factory by top
       * block
       * @param peer_query_factory - factory for peer query component creation
       * @return initialized loader
       */
      auto createLoader(
          std::shared_ptr<ametsuchi::PeerQueryFactory> peer_query_factory);

     public:
      /**
       * Initialize block loader with service and loader
       * @param peer_query_factory - factory to peer query component
       * @param block_query_factory - factory to block query component
       * @param block_cache used to retrieve last block put by consensus
       * @return initialized service
       */
      std::shared_ptr<BlockLoader> initBlockLoader(
          std::shared_ptr<ametsuchi::PeerQueryFactory> peer_query_factory,
          std::shared_ptr<ametsuchi::BlockQueryFactory> block_query_factory,
          std::shared_ptr<consensus::ConsensusResultCache> block_cache);

      std::shared_ptr<BlockLoaderImpl> loader;
      std::shared_ptr<BlockLoaderService> service;
    };
  }  // namespace network
}  // namespace iroha
#endif  // IROHA_BLOCK_LOADER_INIT_HPP
