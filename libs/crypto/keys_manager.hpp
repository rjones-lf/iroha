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

#ifndef IROHA_KEYS_MANAGER_HPP
#define IROHA_KEYS_MANAGER_HPP

#include <string>

#include <boost/optional.hpp>

namespace shared_model {
  namespace crypto {
    class Keypair;
  }
}

namespace iroha {
  /**
   * Interface provides facilities to create and store keypair on disk.
   */
  class KeysManager {
   public:
    virtual ~KeysManager() = default;

    /**
     * Create a new keypair and store it as is on disk
     * @return false if create account failed
     */
    virtual bool createKeys() = 0;

    /**
     * Create keys a new keypair and store it encrypted on disk
     * @param pass_phrase is a password for the keys
     * @return false if create account failed
     */
    virtual bool createKeys(const std::string &pass_phrase) = 0;

    /**
     * Load plain-text keys associated with the manager, then validate loaded
     * keypair by signing and verifying signature of test message
     * @return nullopt if no keypair found locally, or verification failure;
     *         related keypair otherwise
     */
    virtual boost::optional<shared_model::crypto::Keypair> loadKeys() = 0;

    /**
     * Load encrypted keys associated with the manager, then validate loaded
     * keypair by signing and verifying signature of test message
     * @param pass_phrase is a password for decryption
     * @return nullopt if no keypair found locally, or verification failure;
     *         related keypair otherwise
     */
    virtual boost::optional<shared_model::crypto::Keypair> loadKeys(
        const std::string &pass_phrase) = 0;
  };

}  // namespace iroha
#endif  // IROHA_KEYS_MANAGER_HPP
