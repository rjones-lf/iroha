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
        model::ModelCryptoProvider& crypto_provider)
        : crypto_provider_(crypto_provider) {}

    bool ChainValidatorImpl::validateBlock(const model::Block& block,
                                           ametsuchi::MutableStorage& storage) {
      auto apply_block = [](const auto& current_block, auto& executor,
                            auto& query, auto& top_block) {
        for (const auto& tx : current_block.transactions) {
          for (const auto& command : tx.commands) {
            if (not command->execute(query, executor)) {
              return false;
            }
          }
        }
        return true;
      };

      return
          // Check if block has supermajority
          checkSupermajority(storage, block.sigs.size()) &&
          // Verify signatories of the block
          // TODO: use stateful validation here ?
          crypto_provider_.verify(block) &&
          // Apply to temporal storage
          storage.apply(block, apply_block);
    }

    bool ChainValidatorImpl::validateChain(
        rxcpp::observable<model::Block>& blocks,
        ametsuchi::MutableStorage& storage) {
      auto result = false;
      blocks
          .take_while([&result, this, &storage](auto block) {
            return this->validateBlock(block, storage);
          })
          .subscribe([](auto block) {});
      return result;
    }

    bool ChainValidatorImpl::checkSupermajority(
        ametsuchi::MutableStorage& storage, uint64_t signs_num) {
      auto all_peers = storage.getPeers();
      if (not all_peers.has_value()) {
        return false;
      }
      auto f = (all_peers.value().size() - 1) / 3;
      return signs_num >= 2 * f + 1;
    }
  }
}
