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

#include "model/converters/pb_transaction_factory.hpp"
#include "model/commands/add_asset_quantity.hpp"
#include "common/types.hpp"
#include "model/converters/pb_command_factory.hpp"
#include "model/model_hash_provider_impl.hpp"

namespace iroha {
  namespace model {
    namespace converters {

      protocol::Transaction PbTransactionFactory::serialize(
          const model::Transaction &tx) const {
        model::converters::PbCommandFactory factory;
        protocol::Transaction pb_tx;

        // -----|Header|-----
        auto header = pb_tx.mutable_header();
        header->set_created_time(tx.created_ts);
        for (auto &signature : tx.signatures) {
          auto proto_signature = pb_tx.mutable_header()->add_signatures();
          proto_signature->set_pubkey(signature.pubkey.data(),
                                      signature.pubkey.size());
          proto_signature->set_signature(signature.signature.data(),
                                         signature.signature.size());
        }

        // -----|Meta|-----
        auto meta = pb_tx.mutable_meta();
        meta->set_creator_account_id(tx.creator_account_id);
        meta->set_tx_counter(tx.tx_counter);

        // -----|Body|-----
        for (auto &command : tx.commands) {
          auto cmd = pb_tx.mutable_body()->add_commands();
          new (cmd)
              protocol::Command(factory.serializeAbstractCommand(*command));
        }
        return pb_tx;
      }

      std::shared_ptr<model::Transaction> PbTransactionFactory::deserialize(
          const protocol::Transaction &pb_tx) const {
        model::converters::PbCommandFactory commandFactory;
        model::Transaction tx;

        // -----|Header|-----
        tx.created_ts = pb_tx.header().created_time();
        for (auto &pb_sign : pb_tx.header().signatures()) {
          model::Signature sign;
          sign.pubkey = pb_sign.pubkey();
          sign.signature = pb_sign.signature();
          tx.signatures.push_back(std::move(sign));
        }

        // -----|Meta|-----
        tx.creator_account_id = pb_tx.meta().creator_account_id();
        tx.tx_counter = pb_tx.meta().tx_counter();

        // -----|Body|-----
        for (const auto &pb_command : pb_tx.body().commands()) {
          tx.commands.push_back(
              commandFactory.deserializeAbstractCommand(pb_command));
        }

        model::HashProviderImpl hashProvider;
        tx.tx_hash = hashProvider.get_hash(tx);

        return std::make_shared<model::Transaction>(tx);
      }

    }  // namespace converters
  }    // namespace model
}  // namespace iroha
