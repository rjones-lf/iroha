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

#include "index_mediator.hpp"
#include <block.pb.h>

namespace iroha {

  IndexMediator::IndexMediator(ametsuchi::Storage *ametsuchi)
      : ametsuchi_(ametsuchi) {}

  void IndexMediator::synchronize() {
    auto last_block_id_store = ametsuchi_->last_block_id_store();
    auto last_block_id_index_opt = ametsuchi_->last_block_id_index();
    auto last_block_id_index =
        last_block_id_index_opt ? *last_block_id_index_opt : 1;

    for (; last_block_id_index < last_block_id_store; ++last_block_id_index) {
      auto blob = ametsuchi_->get_block(last_block_id_index);
      iroha::protocol::Block block;
      block.ParseFromArray(blob.data(), blob.size());
      ametsuchi_->insert_block_index(last_block_id_index,
                                     std::string());  // TODO calculate hash
      for (int i = 0; i < block.body().txs_size(); ++i) {
        ametsuchi_->insert_tx_index(
            i, std::string(), last_block_id_index);  // TODO calculate hash
      }
    }
  }
}  // namespace iroha