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

#ifndef IROHA_CLI_KEYS_MANAGER_HPP
#define IROHA_CLI_KEYS_MANAGER_HPP

#include <nonstd/optional.hpp>
#include "crypto/crypto.hpp"

namespace iroha {

class KeysManager {
 public:
  /**
   * Load keys associated with account
   * @param account_name
   * @return nullopt if no keypair found locally
   */
  virtual nonstd::optional<iroha::keypair_t> loadKeys() = 0;

  /**
   * Create keys and associate with account
   * @param account_name
   * @param pass_phrase
   * @return false if create account failed
   */
  virtual bool createKeys(std::string pass_phrase) = 0;

};

} // namepsace iroha_cli
#endif  // IROHA_CLI_KEYS_MANAGER_HPP
