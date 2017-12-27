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

#ifndef IROHA_SIGNATURE_HPP
#define IROHA_SIGNATURE_HPP

#include "common/types.hpp"

namespace iroha {
  namespace model {

    /**
     * Signature is a Model structure to store crypto information
     */
    struct Signature {
      Signature() = default;
      Signature(sig_t signature, pubkey_t public_key)
          : signature(signature), pubkey(public_key) {}

      sig_t signature;

      using SignatureType = decltype(signature);

      pubkey_t pubkey;

      using KeyType = decltype(pubkey);

      bool operator==(const Signature& rhs) const;
      bool operator!=(const Signature& rhs) const;
    };
  } // namespace model
} // namespace iroha
#endif  // IROHA_SIGNATURE_HPP
