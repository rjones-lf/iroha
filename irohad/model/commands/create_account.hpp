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

#ifndef IROHA_CREATE_ACCOUNT_HPP
#define IROHA_CREATE_ACCOUNT_HPP

#include <model/command.hpp>

namespace iroha {
  namespace model {

    /**
     * Command for creation of a new account in the system
     */
    struct CreateAccount : public Command {
      /**
       * Account's user name
       */
      std::string account_name;

      /**
       * Account's domain (full name)
       */
      std::string domain_id;

      /**
       * Signatory of account
       */
      pubkey_t pubkey;

      bool operator==(const Command &command) const override;

      CreateAccount() {}

      CreateAccount(const std::string &account_name,
                    const std::string &domain_id,
                    const pubkey_t &pubkey)
          : account_name(account_name), domain_id(domain_id), pubkey(pubkey) {}
    };
  }  // namespace model
}  // namespace iroha
#endif  // IROHA_CREATE_ACCOUNT_HPP
