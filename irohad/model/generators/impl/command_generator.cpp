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

using namespace generator;

namespace iroha {
  namespace model {
    namespace generators {

      std::shared_ptr<Command> CommandGenerator::generateAddPeer(
          std::string address, size_t seed) {
        auto command = std::make_shared<AddPeer>();
        command->address = address;
        command->peer_key = random_blob<ed25519::pubkey_t::size()>(seed);
        return command;
      }

      std::shared_ptr<Command> CommandGenerator::generateCreateAccount(
          std::string account_name, std::string domain_id, size_t seed) {
        auto command = std::make_shared<CreateAccount>();
        command->account_name = account_name;
        command->domain_id = domain_id;
        command->pubkey = random_blob<ed25519::pubkey_t::size()>(seed);
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

    }  // namespace generators
  }    // namespace model
}  // namespace iroha
