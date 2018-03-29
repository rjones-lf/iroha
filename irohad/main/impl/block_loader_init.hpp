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

#include "ametsuchi/block_query.hpp"
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
       * @param storage - used to retrieve blocks
       * @return initialized service
       */
      auto createService(std::shared_ptr<ametsuchi::BlockQuery> storage);

      /**
       * Create block loader for loading blocks from given peer by top block
       * @return initialized loader
       */
      auto createLoader(
          std::shared_ptr<ametsuchi::PeerQuery> peer_query,
          std::shared_ptr<ametsuchi::BlockQuery> storage);

     public:
      /**
       * Initialize block loader with service and loader
       * @return initialized service
       */
      std::shared_ptr<BlockLoader> initBlockLoader(
          std::shared_ptr<ametsuchi::PeerQuery> peer_query,
          std::shared_ptr<ametsuchi::BlockQuery> storage);

      std::shared_ptr<BlockLoaderImpl> loader;
      std::shared_ptr<BlockLoaderService> service;
    };
  }  // namespace network
}  // namespace iroha
#endif  // IROHA_BLOCK_LOADER_INIT_HPP
