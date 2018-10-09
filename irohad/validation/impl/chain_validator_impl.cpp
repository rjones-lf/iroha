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

#include "ametsuchi/mutable_storage.hpp"
#include "ametsuchi/peer_query.hpp"
#include "consensus/yac/supermajority_checker.hpp"
#include "interfaces/iroha_internal/block.hpp"

namespace iroha {
  namespace validation {
    ChainValidatorImpl::ChainValidatorImpl(
        std::shared_ptr<consensus::yac::SupermajorityChecker>
            supermajority_checker)
        : supermajority_checker_(supermajority_checker),
          log_(logger::log("ChainValidator")) {}

    bool ChainValidatorImpl::validateBlock(
        std::shared_ptr<shared_model::interface::Block> block,
        ametsuchi::MutableStorage &storage) const {
      log_->info("validate block: height {}, hash {}",
                 block->height(),
                 block->hash().hex());
      auto check_block =
          [this](const auto &block, auto &queries, const auto &top_hash) {
            auto peers = queries.getLedgerPeers();
            if (not peers) {
              return false;
            }
            return block.prevHash() == top_hash
                and supermajority_checker_->hasSupermajority(block.signatures(),
                                                             peers.value());
          };

      // check inside of temporary storage
      return storage.check(*block, check_block);
    }

    bool ChainValidatorImpl::validateChain(
        rxcpp::observable<std::shared_ptr<shared_model::interface::Block>>
            blocks,
        ametsuchi::MutableStorage &storage) const {
      log_->info("validate chain...");
      return blocks
          .all([this, &storage](auto block) {
            log_->info("Validating block: height {}, hash {}",
                       block->height(),
                       block->hash().hex());
            return this->validateBlock(block, storage);
          })
          .as_blocking()
          .first();
    }

  }  // namespace validation
}  // namespace iroha
