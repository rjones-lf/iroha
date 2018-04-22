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
#include <cryptography/ed25519_sha3_impl/internal/sha3_hash.hpp>
#include "model/commands/add_asset_quantity.hpp"
#include "model/converters/pb_command_factory.hpp"

namespace iroha {
  namespace model {
    namespace converters {

      protocol::Transaction PbTransactionFactory::serialize(
          const model::Transaction &tx) {
        model::converters::PbCommandFactory factory;
        protocol::Transaction pb_tx;

        protocol::Transaction pbtx;

        auto pl = pbtx.mutable_payload();
        pl->set_created_time(tx.created_ts);
        pl->set_creator_account_id(tx.creator_account_id);

        for (const auto &command : tx.commands) {
          auto cmd = pl->add_commands();
          new (cmd)
              protocol::Command(factory.serializeAbstractCommand(*command));
        }

        for (const auto &sig_obj : tx.signatures) {
          auto proto_signature = pbtx.add_signatures();
          proto_signature->set_pubkey(sig_obj.pubkey.to_string());
          proto_signature->set_signature(sig_obj.signature.to_string());
        }
        return pbtx;
      }

      std::shared_ptr<model::Transaction> PbTransactionFactory::deserialize(
          const protocol::Transaction &pb_tx) {
        model::converters::PbCommandFactory commandFactory;
        model::Transaction tx;

        const auto &pl = pb_tx.payload();
        tx.creator_account_id = pl.creator_account_id();
        tx.created_ts = pl.created_time();

        for (const auto &pb_sig : pb_tx.signatures()) {
          model::Signature sig{};
          sig.pubkey = pubkey_t::from_string(pb_sig.pubkey());
          sig.signature = sig_t::from_string(pb_sig.signature());
          tx.signatures.push_back(sig);
        }

        for (const auto &pb_command : pl.commands()) {
          tx.commands.push_back(
              commandFactory.deserializeAbstractCommand(pb_command));
        }

        return std::make_shared<model::Transaction>(tx);
      }

    }  // namespace converters
  }    // namespace model
}  // namespace iroha
