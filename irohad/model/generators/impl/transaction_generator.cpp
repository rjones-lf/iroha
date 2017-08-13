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
        // Create signature
        tx.signatures = {generateSignature(42)};
        tx.creator_account_id = "";
        tx.tx_counter = 0;

        CommandGenerator command_generator;
        // Add peers
        for (size_t i = 0; i < peers_address.size(); ++i) {
          tx.commands.push_back(
              command_generator.generateAddPeer(peers_address[i], i + 1));
        }
        // Add domain
        tx.commands.push_back(command_generator.generateCreateDomain("test"));
        // Create accounts
        tx.commands.push_back(
            command_generator.generateCreateAccount("admin", "test", 1));
        tx.commands.push_back(
            command_generator.generateCreateAccount("test", "test", 2));
        // Create asset
        tx.commands.push_back(
            command_generator.generateCreateAsset("coin", "test", 2));
        // Add admin rights
        tx.commands.push_back(
            command_generator.generateSetAdminPermissions("admin@test"));

        tx.tx_hash = hash_provider_.get_hash(tx);
        return tx;
      }

      Transaction TransactionGenerator::generateTransaction(
          ts64_t timestamp, std::string creator_account_id, uint64_t tx_counter,
          std::vector<std::shared_ptr<Command>> commands) {
        Transaction tx{};
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
