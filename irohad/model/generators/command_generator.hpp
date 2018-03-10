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

#ifndef IROHA_COMMAND_GENERATOR_HPP
#define IROHA_COMMAND_GENERATOR_HPP

#include <memory>
#include "amount/amount.hpp"
#include "generator/generator.hpp"

namespace iroha {

  class Amount;

  namespace model {

    struct Peer;
    struct Command;
    struct Account;

    namespace generators {

      class CommandGenerator {
       public:
        std::shared_ptr<Command> generateAddPeer(const Peer &peer);

        std::shared_ptr<Command> generateAddSignatory(
            const std::string &account_id, const pubkey_t &key);

        std::shared_ptr<Command> generateRemoveSignatory(
            const std::string &account_id, const pubkey_t &key);

        std::shared_ptr<Command> generateCreateAccount(
            const std::string &account_name,
            const std::string &domain_id,
            const pubkey_t &key);

        std::shared_ptr<Command> generateCreateDomain(
            const std::string &domain_id, const std::string &default_role);

        std::shared_ptr<Command> generateCreateAsset(
            const std::string &asset_name,
            const std::string &domain_name,
            uint8_t precision);

        template <typename Type, typename... ParamTypes>
        std::shared_ptr<Command> generateCommand(ParamTypes... args) {
          return std::make_shared<Type>(args...);
        }

        std::shared_ptr<Command> generateCreateAdminRole(std::string role_name);

        std::shared_ptr<Command> generateCreateUserRole(std::string role_name);

        std::shared_ptr<Command> generateCreateAssetCreatorRole(
            std::string role_name);

        std::shared_ptr<Command> generateSetQuorum(
            const std::string &account_id, uint32_t quorum);

        std::shared_ptr<Command> generateAddAssetQuantity(
            const std::string &account_id,
            const std::string &asset_id,
            const Amount &amount);

        std::shared_ptr<Command> generateSubtractAssetQuantity(
            const std::string &account_id,
            const std::string &asset_id,
            const Amount &amount);
        /**
         * Generate transfer assets from source account_id to target account_id
         * @param src_account_id - source account identifier
         * @param target_account_id - target account identifier
         * @param asset_id - asset identifier to transfer
         * @param amount - amount of assets to transfer
         * @return
         */
        std::shared_ptr<Command> generateTransferAsset(
            const std::string &src_account_id,
            const std::string &target_account_id,
            const std::string &asset_id,
            const Amount &amount);

        std::shared_ptr<Command> generateAppendRole(
            const std::string &account_id, const std::string &role_name);
      };
    }  // namespace generators
  }    // namespace model
}  // namespace iroha

#endif  // IROHA_COMMAND_GENERATOR_HPP
