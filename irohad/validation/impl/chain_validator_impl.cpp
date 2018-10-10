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
#include "cryptography/public_key.hpp"
#include "interfaces/common_objects/peer.hpp"
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
              log_->info("Cannot retrieve peers from storage");
              return false;
            }
            auto has_prev_hash = block.prevHash() == top_hash;
            if (not has_prev_hash) {
              log_->info(
                  "Previous hash {} of block does not match top block hash {} "
                  "in storage",
                  block.prevHash().hex(),
                  top_hash.hex());
            }
            auto has_supermajority = supermajority_checker_->hasSupermajority(
                block.signatures(), peers.value());
            if (not has_supermajority) {
              log_->info(
                  "Block does not contain signatures of supermajority of "
                  "peers. Block signatures public keys: [{}], ledger peers "
                  "public keys: [{}]",
                  std::accumulate(std::next(std::begin(block.signatures())),
                                  std::end(block.signatures()),
                                  block.signatures().front().publicKey().hex(),
                                  [](auto acc, auto &sig) {
                                    return acc + ", " + sig.publicKey().hex();
                                  }),
                  std::accumulate(std::next(std::begin(peers.value())),
                                  std::end(peers.value()),
                                  peers.value().front()->pubkey().hex(),
                                  [](auto acc, auto &peer) {
                                    return acc + ", " + peer->pubkey().hex();
                                  }));
            }
            return has_prev_hash and has_supermajority;
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
