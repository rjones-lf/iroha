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

#include "model/generators/command_generator.hpp"
#include "common/types.hpp"
#include "model/commands/add_asset_quantity.hpp"
#include "model/commands/add_peer.hpp"
#include "model/commands/create_account.hpp"
#include "model/commands/create_asset.hpp"
#include "model/commands/create_domain.hpp"
#include "model/commands/set_permissions.hpp"
#include "model/commands/set_quorum.hpp"
#include "model/commands/subtract_asset_quantity.hpp"
#include "model/commands/transfer_asset.hpp"

using namespace generator;

namespace iroha {
  namespace model {
    namespace generators {

      std::shared_ptr<Command> CommandGenerator::generateAddPeer(
          std::string address, ed25519::pubkey_t key) {
        auto command = std::make_shared<AddPeer>();
        command->address = address;
        command->peer_key = key;
        return command;
      }

      std::shared_ptr<Command> CommandGenerator::generateCreateAccount(
          std::string account_name, std::string domain_id,
          ed25519::pubkey_t key) {
        auto command = std::make_shared<CreateAccount>();
        command->account_name = account_name;
        command->domain_id = domain_id;
        command->pubkey = key;
        return command;
      }

      std::shared_ptr<Command> CommandGenerator::generateCreateDomain(
          std::string domain_name) {
        auto command = std::make_shared<CreateDomain>();
        command->domain_name = domain_name;
        return command;
      }

      std::shared_ptr<Command> CommandGenerator::generateCreateAsset(
          std::string asset_name, std::string domain_name, uint8_t precision) {
        auto command = std::make_shared<CreateAsset>();
        command->domain_id = domain_name;
        command->asset_name = asset_name;
        command->precision = precision;
        return command;
      }

      std::shared_ptr<Command> CommandGenerator::generateSetAdminPermissions(
          std::string account_id) {
        auto command = std::make_shared<SetAccountPermissions>();
        command->account_id = account_id;
        Account::Permissions permissions;
        permissions.read_all_accounts = true;
        permissions.set_permissions = true;
        permissions.issue_assets = true;
        permissions.can_transfer = true;
        command->new_permissions = permissions;
        return command;
      }

      std::shared_ptr<Command> CommandGenerator::generateAddAssetQuantity(
          std::string account_id, std::string asset_id, Amount amount) {
        auto command = std::make_shared<AddAssetQuantity>();
        command->account_id = account_id;
        command->asset_id = asset_id;
        command->amount = amount;
        return command;
      }

      std::shared_ptr<Command> CommandGenerator::generateSetQuorum(
          std::string account_id, uint32_t quorum) {
        auto command = std::make_shared<SetQuorum>();
        command->account_id = account_id;
        command->new_quorum = quorum;
        return command;
      }

      std::shared_ptr<Command> CommandGenerator::generateSubtractAssetQuantity(
          std::string account_id, std::string asset_id, Amount amount) {
        // TODO: implement
        return nullptr;
      }

      std::shared_ptr<Command> CommandGenerator::generateTransferAsset(std::string src_account,
                                                                       std::string dest_account,
                                                                       std::string asset_id,
                                                                       Amount amount) {
        auto command = std::make_shared<TransferAsset>();
        command->amount = amount;
        command->asset_id = asset_id;
        command->src_account_id = src_account;
        command->dest_account_id = dest_account;
        return command;
      }


    }  // namespace generators
  }    // namespace model
}  // namespace iroha
