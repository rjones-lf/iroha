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

#ifndef IROHA_DETACH_ROLE_HPP
#define IROHA_DETACH_ROLE_HPP

#include "model/command.hpp"

namespace iroha {
  namespace model {

    /**
     * Create new role in the system
     */
    struct DetachRole : public Command {
      /**
       * Account from where detach role
       */
      std::string account_id;
      /**
       * Role to detach
       */
      std::string role_name;

      bool operator==(const Command &command) const override;

      DetachRole() {}
      /**
       * @param role_name_ - name of the role in the system
       * @param perms - set of permissions for the role
       */
      DetachRole(const std::string &account_id, const std::string &role_name_)
          : account_id(account_id), role_name(role_name_) {}
    };
  }  // namespace model
}  // namespace iroha

#endif  // IROHA_DETACH_ROLE_HPP
