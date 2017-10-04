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

#include "crypto/hash.hpp"
#include "crypto/keys_manager_impl.hpp"
#include "datetime/time.hpp"

namespace iroha {
  namespace model {
    namespace generators {
      Transaction TransactionGenerator::generateGenesisTransaction(
          ts64_t timestamp,
          std::vector<std::string> peers_address,
          std::vector<pubkey_t> public_keys) {
        Transaction tx;
        tx.created_ts = timestamp;
        tx.creator_account_id = "";
        tx.tx_counter = 0;

        CommandGenerator command_generator;
        // Add peers
        for (size_t i = 0; i < peers_address.size(); ++i) {
          auto peer_key = public_keys.at(i);
          tx.commands.push_back(
              command_generator.generateAddPeer(peers_address[i], peer_key));
        }
        // Add domain
        tx.commands.push_back(command_generator.generateCreateDomain("test"));
        // Create accounts
        KeysManagerImpl manager("admin@test");
        manager.createKeys("admin@test");
        auto keypair = *manager.loadKeys();
        tx.commands.push_back(command_generator.generateCreateAccount(
            "admin", "test", keypair.pubkey));
        manager = KeysManagerImpl("test@test");
        manager.createKeys("test@test");
        keypair = *manager.loadKeys();
        tx.commands.push_back(command_generator.generateCreateAccount(
            "test", "test", keypair.pubkey));
        // Create asset
        auto precision = 2;
        tx.commands.push_back(
            command_generator.generateCreateAsset("coin", "test", precision));
        // Add admin rights
        tx.commands.push_back(
            command_generator.generateSetAdminPermissions("admin@test"));

        return tx;
      }

      Transaction TransactionGenerator::generateGenesisTransaction(
          ts64_t timestamp, std::vector<std::string> peers_address) {
        std::vector<pubkey_t> public_keys;
        for (size_t i = 0; i < peers_address.size(); ++i) {
          KeysManagerImpl manager("node" + std::to_string(i));
          manager.createKeys("node" + std::to_string(i));
          auto keypair = *manager.loadKeys();
          public_keys.push_back(keypair.pubkey);
        }
        return generateGenesisTransaction(
            timestamp, peers_address, public_keys);
      }

      Transaction TransactionGenerator::generateTransaction(
          ts64_t timestamp,
          std::string creator_account_id,
          uint64_t tx_counter,
          std::vector<std::shared_ptr<Command>> commands) {
        Transaction tx;
        tx.created_ts = timestamp;
        tx.creator_account_id = creator_account_id;
        tx.tx_counter = tx_counter;
        tx.commands = commands;
        return tx;
      }

      Transaction TransactionGenerator::generateTransaction(
          std::string creator_account_id,
          uint64_t tx_counter,
          std::vector<std::shared_ptr<Command>> commands) {
        return generateTransaction(
            iroha::time::now(), creator_account_id, tx_counter, commands);
      }

    }  // namespace generators
  }    // namespace model
}  // namespace iroha
