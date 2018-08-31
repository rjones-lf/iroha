/**
 * Copyright Soramitsu Co., Ltd. 2017 All Rights Reserved.
 * http://soramitsu.co.jp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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
       * @param peer_query_factory - factory to peer query component
       * @param block_query_factory - factory to block query component
       * @return initialized loader
       */
      auto createLoader(std::shared_ptr<ametsuchi::PeerQueryFactory> peer_query_factory,
                        std::shared_ptr<ametsuchi::BlockQueryFactory> block_query_factory);

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
