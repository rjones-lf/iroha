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
#ifndef IROHA_REMOVE_SIGNATORY_HPP
#define IROHA_REMOVE_SIGNATORY_HPP

#include <string>
#include "common/types.hpp"
#include "model/command.hpp"

namespace iroha {
  namespace model {

    /**
     * Attach signatory for account
     */
    struct RemoveSignatory : public Command {
      /**
       * Account to remove from
       */
      std::string account_id;

      /**
       * Public key of signatory to remove.
       * Note: This public key must be attach to account.
       * There must be at least two signatories to perform this operation.
       */
      pubkey_t pubkey;

      bool operator==(const Command &command) const override;

      RemoveSignatory() {}

      RemoveSignatory(const std::string &account_id, const pubkey_t &pubkey)
          : account_id(account_id), pubkey(pubkey) {}
    };
  }  // namespace model
}  // namespace iroha

#endif  // IROHA_REMOVE_SIGNATORY_HPP
