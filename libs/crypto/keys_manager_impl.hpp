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

#ifndef IROHA_CLI_KEYS_MANAGER_IMPL_HPP
#define IROHA_CLI_KEYS_MANAGER_IMPL_HPP

#include "crypto/keys_manager.hpp"

#include <boost/optional.hpp>

#include "common/types.hpp"  // for keypair_t, pubkey_t, privkey_t
#include "logger/logger.hpp"

namespace iroha {

  class KeysManagerImpl : public KeysManager {
   public:
    explicit KeysManagerImpl(const std::string &account_name);

    boost::optional<iroha::keypair_t> loadKeys() override;
    boost::optional<iroha::keypair_t> loadKeys(
        const std::string &pass_phrase) override;

    bool createKeys() override;
    bool createKeys(const std::string &pass_phrase) override;

    static const std::string kPubExt;
    static const std::string kPrivExt;

   private:
    /**
     * Check if keypair provides valid signature
     * @param keypair - keypair for validation
     * @return true, if verification of signature is successful
     */
    bool validate(const iroha::keypair_t &keypair) const;

    /**
     * Tries to load file to the resulting string
     * @param filename file to read from
     * @param res is where result is stored
     * @return true, if no problem with file reading
     */
    bool loadFile(const std::string &filename, std::string &res);

    /**
     * Stores strings, that represent public and private keys on disk
     * @param pub is a public key
     * @param priv is a private key
     * @return true, if saving was successful
     */
    bool store(const std::string &pub, const std::string &priv);

    std::string account_name_;
    logger::Logger log_;
  };
}  // namespace iroha
#endif  // IROHA_CLI_KEYS_MANAGER_IMPL_HPP
