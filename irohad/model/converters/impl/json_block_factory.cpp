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

#include "model/converters/json_block_factory.hpp"
#include "model/converters/json_common.hpp"

using namespace rapidjson;

namespace iroha {
  namespace model {
    namespace converters {

      JsonBlockFactory::JsonBlockFactory() {
        log_ = logger::log("JsonBlockFactory");
      }

      Document JsonBlockFactory::serialize(const Block &block) {
        Document document;
        auto &allocator = document.GetAllocator();
        document.SetObject();

        Value signatures;
        signatures.SetArray();
        for (const auto &signature : block.sigs) {
          signatures.PushBack(serializeSignature(signature, allocator), allocator);
        }
        document.AddMember("signatures", signatures, allocator);

        document.AddMember("created_ts", block.created_ts, allocator);
        document.AddMember("hash", block.hash.to_hexstring(), allocator);
        document.AddMember("prev_hash", block.prev_hash.to_hexstring(),
                           allocator);
        document.AddMember("height", block.height, allocator);
        document.AddMember("txs_number", block.txs_number, allocator);
        document.AddMember("merkle_root", block.merkle_root.to_hexstring(),
                           allocator);

        Value commands;
        commands.SetArray();
        for (auto &&transaction : block.transactions) {
          commands.PushBack(
              Document(&allocator)
                  .CopyFrom(factory_.serialize(transaction), allocator),
              allocator);
        }
        document.AddMember("transactions", commands, allocator);

        return document;
      }

      nonstd::optional<Block> JsonBlockFactory::deserialize(
          const Document &document) {
        auto des = makeFieldDeserializer(document);
        auto des_transactions = [this](auto array) {
          auto acc_transactions = [this](auto init, auto &x) {
            return init
                | [this, &x](auto transactions) {
                    return factory_.deserialize(x)
                        | [&transactions](auto transaction) {
                            transactions.push_back(transaction);
                            return nonstd::make_optional(transactions);
                          };
                  };
          };
          return std::accumulate(
              array.begin(), array.end(),
              nonstd::make_optional<Block::TransactionsType>(),
              acc_transactions);
        };
        return nonstd::make_optional<model::Block>()
            | des.Uint64(&Block::created_ts, "created_ts")
            | des.Uint64(&Block::height, "height")
            | des.Uint(&Block::txs_number, "txs_number")
            | des.String(&Block::hash, "hash")
            | des.String(&Block::prev_hash, "prev_hash")
            | des.String(&Block::merkle_root, "merkle_root")
            | des.Array(&Block::sigs, "signatures")
            | des.Array(&Block::transactions, "transactions", des_transactions);
      }

    }  // namespace converters
  }    // namespace model
}  // namespace iroha
