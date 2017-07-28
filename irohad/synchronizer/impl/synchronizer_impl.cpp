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

#include "synchronizer/impl/synchronizer_impl.hpp"


namespace iroha {
  namespace synchronizer {

    SynchronizerImpl::SynchronizerImpl(
        validation::ChainValidator &validator,
        ametsuchi::MutableFactory &mutableFactory,
        network::BlockLoader &blockLoader)
        : validator_(validator),
          mutableFactory_(mutableFactory),
          blockLoader_(blockLoader) {}

    void SynchronizerImpl::process_commit(iroha::model::Block commit_message) {
      auto storage = mutableFactory_.createMutableStorage();
      if (not storage) {
        // TODO: write to log ametsuchi is not ok
        return;
      }
      if (validator_.validateBlock(commit_message, *storage)) {
        // Block can be applied to current storage
        // Commit to main Ametsuchi
        mutableFactory_.commit(std::move(storage));

        auto single_commit = rxcpp::observable<>::create<model::Block>(
            [&commit_message](auto s) {
              s.on_next(commit_message);
              s.on_completed();
            });

        notifier_.get_subscriber().on_next(single_commit);
      } else {
        // Block can't be applied to current storage
        // Download all missing blocks
        // TODO: Loading blocks from other Peer
        // TODO: Replace with more effective realization
        for (auto signature : commit_message.sigs) {
          auto target_peer = model::Peer();
          target_peer.pubkey = signature.pubkey;

          // Get your last top block
          auto top_block = model::Block();
          auto chain = blockLoader_.requestBlocks(target_peer, top_block);
          storage = mutableFactory_.createMutableStorage();
          if (!storage) {
            // TODO: write to log, cant create storage
            return;
          }
          if (validator_.validateChain(chain, *storage)) {
            // Peer send valid chain
            mutableFactory_.commit(std::move(storage));
            notifier_.get_subscriber().on_next(chain);
            // You are synchonized
            return;
          }
        }
      }
    }

    rxcpp::observable<Commit> SynchronizerImpl::on_commit_chain() {
      return notifier_.get_observable();
    }
  }
}
