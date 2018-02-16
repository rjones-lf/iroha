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
#ifndef IROHA_CREATE_DOMAIN_HPP
#define IROHA_CREATE_DOMAIN_HPP

#include "model/command.hpp"

namespace iroha {
  namespace model {

    /**
     * Create new asset in the system
     */
    struct CreateDomain : public Command {
      /**
       * Asset to insert to the system
       */
      std::string domain_id;

      /**
       * Default role for users in the domain
       */
      std::string user_default_role;

      bool operator==(const Command &command) const override;

      CreateDomain() {}

      /**
       * @param domain_id - id of the domain to create
       * @param user_default_role - default role of the user in this domain
       */
      CreateDomain(const std::string &domain_id,
                   const std::string &user_default_role)
          : domain_id(domain_id), user_default_role(user_default_role) {}
    };
  }  // namespace model
}  // namespace iroha
#endif  // IROHA_CREATE_DOMAIN_HPP
