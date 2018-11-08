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

#include "cryptography/ed25519_sha3_impl/signer.hpp"
#include "crypto/hash_types.hpp"
#include "cryptography/ed25519_sha3_impl/internal/ed25519_impl.hpp"
#include "cryptography/ed25519_sha3_impl/internal/sha3_hash.hpp"

namespace shared_model {
  namespace crypto {
    Signed Signer::sign(const Blob &blob, const Keypair &keypair) {
      return Signed(
          iroha::sign(
              iroha::sha3_256(crypto::toBinaryString(blob)).to_string(),
              iroha::pubkey_t::from_string(toBinaryString(keypair.publicKey())),
              iroha::privkey_t::from_string(
                  toBinaryString(keypair.privateKey())))
              .to_string());
    }
  }  // namespace crypto
}  // namespace shared_model
