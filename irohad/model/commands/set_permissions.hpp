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

#ifndef IROHA_SET_PERMISSIONS_HPP
#define IROHA_SET_PERMISSIONS_HPP

#include <model/account.hpp>
#include <model/command.hpp>
#include <string>

namespace iroha {
  namespace model {
    /**
     * Set permissions for account
     */
    struct SetAccountPermissions : public Command {
      /**
       * Identifier of account to set permission
       */
      std::string account_id;

      /**
       * New permissions of account
       */
      Account::Permissions new_permissions;

      bool operator==(const Command& command) const override;
      bool operator!=(const Command& command) const override;

      SetAccountPermissions() {}

      SetAccountPermissions(std::string account_id,
                            Account::Permissions new_permissions)
          : account_id(account_id), new_permissions(new_permissions) {}
    };
  }  // namespace model
}  // namespace iroha
#endif  // IROHA_SET_PERMISSIONS_HPP
