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

#ifndef IROHA_KEYPAIR_HPP_
#define IROHA_KEYPAIR_HPP_

#include <crypto/crypto.hpp>
#include "private_key.hpp"

namespace iroha {
  namespace crypto {

    using PublicKey = std::vector<uint8_t>;

    /**
     * @class Keypair
     * @brief Represents a keypair of given type.
     */
    class Keypair {
     public:
      Keypair() = delete;
      Keypair(Keypair const&) = delete;
      Keypair(Keypair&&) noexcept = default;
      Keypair& operator=(Keypair const&) = delete;
      Keypair& operator=(Keypair&&) noexcept = default;

      enum class Type { ed25519 };

      static Keypair random(Type t) {
        if (t == Type::ed25519) {
          auto kp = create_keypair(create_seed());
        }
      }

     private:
      PrivateKey priv;
      PublicKey pub;
    };

  }  // namespace crypto
}  // namespace iroha

#endif  //  IROHA_KEYPAIR_HPP_
