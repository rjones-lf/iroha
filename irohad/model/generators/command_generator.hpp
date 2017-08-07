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

#include "model/command.hpp"
#include "model/commands/add_peer.hpp"
#include "model/commands/create_account.hpp"
#include "model/commands/create_asset.hpp"
#include "model/commands/create_domain.hpp"
#include "model/commands/set_permissions.hpp"

#include <memory>
#include "generator/generator.hpp"

namespace iroha {
  namespace model {
    namespace generators {
      class CommandGenerator {
       public:
        /**
         *
         * @param address
         * @param seed
         * @return
         */
        std::shared_ptr<Command> generateAddPeer(std::string address,
                                                 size_t seed);

        /**
         *
         * @param account_name
         * @param domain_id
         * @param seed
         * @return
         */
        std::shared_ptr<Command> generateCreateAccount(std::string account_name,
                                                       std::string domain_id,
                                                       size_t seed);
        /**
         *
         * @param domain_name
         * @return
         */
        std::shared_ptr<Command> generateCreateDomain(std::string domain_name);

        /**
         *
         * @param asset_name
         * @param domain_name
         * @param precision
         * @return
         */
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
      };
    }  // namespace generators
  }    // namespace model
}  // namespace iroha

#endif  // IROHA_COMMAND_GENERATOR_HPP
