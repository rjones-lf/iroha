/**
 * Copyright Soramitsu Co., Ltd. 2018 All Rights Reserved.
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

#include "wsv_restorer_impl.hpp"

#include <vector>

#include "ametsuchi/block_query.hpp"
#include "ametsuchi/storage.hpp"
#include "model/block.hpp"

namespace iroha {
  namespace ametsuchi {
    expected::Result<void, std::string> WsvRestorerImpl::restoreWsv(
        Storage &storage) {
      // get all blocks starting from the genesis
      std::vector<std::shared_ptr<shared_model::interface::Block>> blocks;
      storage.getBlockQuery()->getBlocksFrom(1).as_blocking().subscribe(
          [&blocks](auto block) {
            blocks.push_back(std::move(block));
          });

      storage.dropStorage();

      if (not storage.insertBlocks(blocks))
        return expected::makeError("cannot insert blocks");

      return expected::Value<void>();
    }
  }  // namespace ametsuchi
}  // namespace iroha
