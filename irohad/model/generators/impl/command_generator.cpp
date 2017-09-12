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
#include "model/commands/add_asset_quantity.hpp"
#include "model/commands/add_peer.hpp"
#include "model/commands/add_signatory.hpp"
#include "model/commands/assign_master_key.hpp"
#include "model/commands/create_account.hpp"
#include "model/commands/create_asset.hpp"
#include "model/commands/create_domain.hpp"
#include "model/commands/remove_signatory.hpp"
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
        return generateCommand<AddPeer>(key, address);
      }

      std::shared_ptr<Command> CommandGenerator::generateAddSignatory(
          std::string account_id, ed25519::pubkey_t key) {
        return generateCommand<AddSignatory>(account_id, key);
      }

      std::shared_ptr<Command> CommandGenerator::generateRemoveSignatory(
          std::string account_id, ed25519::pubkey_t key) {
        return generateCommand<RemoveSignatory>(account_id, key);
      }

      std::shared_ptr<Command> CommandGenerator::generateAssignMasterKey(
          std::string account_id, ed25519::pubkey_t key) {
        return generateCommand<AssignMasterKey>(account_id, key);
      }

      std::shared_ptr<Command> CommandGenerator::generateCreateAccount(
          std::string account_name, std::string domain_id,
          ed25519::pubkey_t key) {
        return generateCommand<CreateAccount>(account_name, domain_id, key);
      }

      std::shared_ptr<Command> CommandGenerator::generateCreateDomain(
          std::string domain_name) {
        return generateCommand<CreateDomain>(domain_name);
      }

      std::shared_ptr<Command> CommandGenerator::generateCreateAsset(
          std::string asset_name, std::string domain_name, uint8_t precision) {
        return generateCommand<CreateAsset>(asset_name, domain_name, precision);
      }

      std::shared_ptr<Command> CommandGenerator::generateSetAdminPermissions(
          std::string account_id) {
        Account::Permissions permissions;
        permissions.read_all_accounts = true;
        permissions.set_permissions = true;
        permissions.issue_assets = true;
        permissions.can_transfer = true;
        return generateCommand<SetAccountPermissions>(account_id, permissions);
      }

      std::shared_ptr<Command> CommandGenerator::generateAddAssetQuantity(
          std::string account_id, std::string asset_id, Amount amount) {
        return generateCommand<AddAssetQuantity>(account_id, asset_id, amount);
      }

      std::shared_ptr<Command> CommandGenerator::generateSetQuorum(
          std::string account_id, uint32_t quorum) {
        return generateCommand<SetQuorum>(account_id, quorum);
      }

      std::shared_ptr<Command> CommandGenerator::generateSetPermissions(
          std::string account_id, Account::Permissions permissions) {
        return generateCommand<SetAccountPermissions>(account_id, permissions);
      }

      std::shared_ptr<Command> CommandGenerator::generateSubtractAssetQuantity(
          std::string account_id, std::string asset_id, Amount amount) {
        // TODO: implement
        return nullptr;
      }

      std::shared_ptr<Command> CommandGenerator::generateTransferAsset(
          std::string src_account, std::string dest_account,
          std::string asset_id, Amount amount) {
        return generateCommand<TransferAsset>(src_account, dest_account,
                                              asset_id, amount);
      }

    }  // namespace generators
  }    // namespace model
}  // namespace iroha
