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

#include "common/byteutils.hpp"
#include "common/types.hpp"
#include "crypto/keys_manager.hpp"
#include "logger/logger.hpp"

namespace iroha {
  class KeysManagerImpl : public KeysManager {
   public:
    explicit KeysManagerImpl(std::string account_name);

    nonstd::optional<iroha::keypair_t> loadKeys() override;

    bool createKeys(std::string pass_phrase) override;

   private:

    /**
     * Check if keypair provides valid signature
     * @param keypair - keypair for validation
     * @return true, if verification of signature is successful
     */
    bool validate(const iroha::keypair_t &keypair) const;

    std::string account_name_;
    logger::Logger log_;
  };
}  // namespace iroha_cli
#endif  // IROHA_CLI_KEYS_MANAGER_IMPL_HPP
