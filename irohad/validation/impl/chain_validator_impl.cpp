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
#include "backend/protobuf/from_old_model.hpp"

#include "ametsuchi/impl/postgres_wsv_query.hpp"
#include "consensus/yac/supermajority_checker.hpp"

namespace iroha {
  namespace validation {
    ChainValidatorImpl::ChainValidatorImpl(
        std::shared_ptr<consensus::yac::SupermajorityChecker>
            supermajority_checker)
        : supermajority_checker_(supermajority_checker) {
      log_ = logger::log("ChainValidator");
    }

    bool ChainValidatorImpl::validateBlock(const model::Block &block,
                                           ametsuchi::MutableStorage &storage) {
      log_->info("validate block: height {}, hash {}",
                 block.height,
                 block.hash.to_hexstring());
      auto apply_block =
          [this](const auto &block, auto &queries, const auto &top_hash) {
            auto peers = queries.getPeers();
            if (not peers.has_value()) {
              return false;
            }
            auto old_block = *std::unique_ptr<model::Block>(block.makeOldModel());
            return block.prevHash() == top_hash
                and supermajority_checker_->hasSupermajority(old_block.sigs,
                                                             peers.value());
          };

      // Apply to temporary storage
      auto old_block = shared_model::proto::from_old(block);
      return storage.apply(old_block, apply_block);
    }

    bool ChainValidatorImpl::validateChain(OldCommit blocks,
                                           ametsuchi::MutableStorage &storage) {
      log_->info("validate chain...");
      return blocks
          .all([this, &storage](auto block) {
            log_->info("Validating block: height {}, hash {}",
                       block.height,
                       block.hash.to_hexstring());
            return this->validateBlock(block, storage);
          })
          .as_blocking()
          .first();
    }
  }  // namespace validation
}  // namespace iroha
