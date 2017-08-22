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
#include "generator/generator.hpp"
#include "model/command.hpp"

namespace iroha {
  namespace model {
    namespace generators {
      class CommandGenerator {
       public:
        std::shared_ptr<Command> generateAddPeer(std::string address,
                                                 ed25519::pubkey_t key);

        std::shared_ptr<Command> generateCreateAccount(std::string account_name,
                                                       std::string domain_id,
                                                       ed25519::pubkey_t key);

        std::shared_ptr<Command> generateCreateDomain(std::string domain_name);

        std::shared_ptr<Command> generateCreateAsset(std::string asset_name,
                                                     std::string domain_name,
                                                     uint8_t precision);

        /**
         *
         * @param account_id
         * @return
         */
        std::shared_ptr<Command> generateSetAdminPermissions(
            std::string account_id);

        std::shared_ptr<Command> generateSetQuorum(std::string account_id,
                                                   uint32_t quorum);

        std::shared_ptr<Command> generateAddAssetQuantity(
            std::string account_id, std::string asset_id, Amount amount);

        std::shared_ptr<Command> generateSubtractAssetQuantity(
            std::string account_id, std::string asset_id, Amount amount);

        std::shared_ptr<Command> generateTransferAsset(
            std::string src_account, std::string target_account,
            std::string asset_id, Amount amount);
      };
    }  // namespace generators
  }    // namespace model
}  // namespace iroha

#endif  // IROHA_COMMAND_GENERATOR_HPP
