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

#include "main/impl/block_loader_init.hpp"
#include "validators/default_validator.hpp"

using namespace iroha;
using namespace iroha::ametsuchi;
using namespace iroha::network;

auto BlockLoaderInit::createService(std::shared_ptr<BlockQuery> storage) {
  return std::make_shared<BlockLoaderService>(storage);
}

auto BlockLoaderInit::createLoader(
    std::shared_ptr<PeerQuery> peer_query,
    std::shared_ptr<BlockQuery> storage) {
  return std::make_shared<BlockLoaderImpl>(
      peer_query,
      storage,
      std::make_shared<shared_model::validation::DefaultBlockValidator>());
}

std::shared_ptr<BlockLoader> BlockLoaderInit::initBlockLoader(
    std::shared_ptr<PeerQuery> peer_query,
    std::shared_ptr<BlockQuery> storage) {
  service = createService(storage);
  loader = createLoader(peer_query, storage);
  return loader;
}
