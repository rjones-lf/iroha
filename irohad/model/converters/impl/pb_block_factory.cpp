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

#include "model/converters/pb_block_factory.hpp"
#include "model/converters/pb_common.hpp"
#include "model/converters/pb_transaction_factory.hpp"

#include "crypto/hash.hpp"

namespace iroha {
  namespace model {
    namespace converters {

      protocol::Block PbBlockFactory::serialize(
          const model::Block& block) const {
        protocol::Block pb_block{};

        auto pl = pb_block.mutable_payload();
        pl->set_tx_number(block.txs_number);
        pl->set_height(block.height);
        pl->set_merkle_root(block.merkle_root.to_string());
        pl->set_prev_block_hash(block.prev_hash.to_string());
        pl->set_created_time(block.created_ts);

        for (const auto& sig_obj : block.sigs) {
          auto sig = pb_block.add_signatures();
          sig->set_pubkey(sig_obj.pubkey.to_string());
          sig->set_signature(sig_obj.signature.to_string());
        }

        for (const auto& tx : block.transactions) {
          auto pb_tx = pl->add_transactions();
          pb_tx->CopyFrom(PbTransactionFactory::serialize(tx));
        }

        return pb_block;
      }

      model::Block PbBlockFactory::deserialize(
          protocol::Block const& pb_block) const {
        model::Block block{};
        const auto& pl = pb_block.payload();

        // in proto we use uint32, but txs_number is uint16
        auto txn_max = std::numeric_limits<decltype(block.txs_number)>::max();
        if (pl.tx_number() > txn_max) {
          throw BadFormatException("too many transactions in block");
        }

        block.txs_number = static_cast<uint16_t>(pl.tx_number());
        block.height = pl.height();
        block.merkle_root = hash256_t::from_string(pl.merkle_root());
        block.prev_hash = hash256_t::from_string(pl.prev_block_hash());
        block.created_ts = pl.created_time();

        for (const auto& pb_sig : pb_block.signatures()) {
          model::Signature sig;
          sig.signature = sig_t::from_string(pb_sig.signature());
          sig.pubkey = pubkey_t::from_string(pb_sig.pubkey());
          block.sigs.push_back(std::move(sig));
        }

        for (const auto& pb_tx : pl.transactions()) {
          block.transactions.push_back(
              *PbTransactionFactory::deserialize(pb_tx));
        }

        block.hash = iroha::hash(pb_block);

        return block;
      }
    }  // namespace converters
  }    // namespace model
}  // namespace iroha
