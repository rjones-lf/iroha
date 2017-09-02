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

#include "validation/impl/chain_validator_impl.hpp"

namespace iroha {
  namespace validation {

    ChainValidatorImpl::ChainValidatorImpl(
        std::shared_ptr<model::ModelCryptoProvider> crypto_provider)
        : crypto_provider_(crypto_provider) {
      log_ = logger::log("ChainValidator");
    }

    bool ChainValidatorImpl::validateBlock(const model::Block &block,
                                           ametsuchi::MutableStorage &storage) {
      log_->info("validate block");
      auto apply_block = [](const auto &current_block,
                            auto &query, const auto &top_hash) {
        return current_block.prev_hash == top_hash;
      };

      return
        // Check if block has supermajority
          checkSupermajority(storage, block.sigs.size()) &&
              // Verify signatories of the block
              // TODO: use stateful validation here ?
              crypto_provider_->verify(block) &&
              // Apply to temporary storage
              storage.apply(block, apply_block);
    }

    bool ChainValidatorImpl::validateChain(Commit blocks,
                                           ametsuchi::MutableStorage &storage) {
      log_->info("validate chain...");
      return blocks
          .all([this, &storage](auto block) {
            return this->validateBlock(block, storage);
          })
          .as_blocking()
          .first();
    }

    bool ChainValidatorImpl::checkSupermajority(
        ametsuchi::MutableStorage &storage, uint64_t signs_num) {
      auto all_peers = storage.getPeers();
      if (not all_peers.has_value()) {
        return false;
      }
      int64_t all = all_peers.value().size();
      auto f = (all - 1) / 3.0;
      return signs_num >= 2 * f + 1;
    }
  }
}
