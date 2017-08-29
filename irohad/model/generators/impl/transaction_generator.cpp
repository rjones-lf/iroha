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

#include "model/generators/transaction_generator.hpp"

namespace iroha {
  namespace model {
    namespace generators {
      Transaction TransactionGenerator::generateGenesisTransaction(
          ts64_t timestamp, std::vector<std::string> peers_address) {
        Transaction tx;
        tx.created_ts = timestamp;
        tx.creator_account_id = "";
        tx.tx_counter = 0;

        CommandGenerator command_generator;
        // Add peers
        for (size_t i = 0; i < peers_address.size(); ++i) {
          // TODO: replace with more flexible scheme, generate public keys with
          // specified parameters
          auto peer_key =
              generator::random_blob<ed25519::pubkey_t::size()>(i + 1);
          tx.commands.push_back(
              command_generator.generateAddPeer(peers_address[i], peer_key));
        }
        // Add domain
        tx.commands.push_back(command_generator.generateCreateDomain("test"));
        // Create accounts
        auto acc_key = generator::random_blob<ed25519::pubkey_t::size()>(1);
        tx.commands.push_back(
            command_generator.generateCreateAccount("admin", "test", acc_key));
        acc_key = generator::random_blob<ed25519::pubkey_t::size()>(2);
        tx.commands.push_back(
            command_generator.generateCreateAccount("test", "test", acc_key));
        // Create asset
        auto precision = 2;
        tx.commands.push_back(
            command_generator.generateCreateAsset("coin", "test", precision));
        // Add admin rights
        tx.commands.push_back(
            command_generator.generateSetAdminPermissions("admin@test"));

        tx.tx_hash = hash_provider_.get_hash(tx);
        return tx;
      }

      Transaction TransactionGenerator::generateTransaction(
          ts64_t timestamp, std::string creator_account_id, uint64_t tx_counter,
          std::vector<std::shared_ptr<Command>> commands) {
        Transaction tx;
        tx.created_ts = timestamp;
        tx.creator_account_id = creator_account_id;
        tx.tx_counter = tx_counter;
        tx.commands = commands;
        tx.tx_hash = hash_provider_.get_hash(tx);
        return tx;
      }

    }  // namespace generators
  }    // namespace model
}  // namespace iroha
