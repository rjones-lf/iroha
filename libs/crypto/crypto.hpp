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

#ifndef IROHA_CRYPTO_HPP
#define IROHA_CRYPTO_HPP

#include <common/types.hpp>
#include <string>

namespace iroha {

  /**
   * Sign message with ed25519 crypto algorithm
   * @param msg
   * @param msgsize
   * @param pub
   * @param priv
   * @return
   */
  ed25519::sig_t sign(const uint8_t *msg, size_t msgsize,
                      const ed25519::pubkey_t &pub,
                      const ed25519::privkey_t &priv);

  /**
   * Verify signature of ed25519 crypto algorithm
   * @param msg
   * @param msgsize
   * @param pub
   * @param sig
   * @return true if signature is valid, false otherwise
   */
  bool verify(const uint8_t *msg, size_t msgsize, const ed25519::pubkey_t &pub,
              const ed25519::sig_t &sig);

  /**
   * Generate random seed reading from /dev/urandom
   */
  blob_t<32> create_seed();

  /**
   * Generate random seed as sha3_256(passphrase)
   * @param passphrase
   * @return
   */
  blob_t<32> create_seed(std::string passphrase);

  /**
   * Create new keypair
   * @param seed
   * @return
   */
  ed25519::keypair_t create_keypair(blob_t<32> seed);


}
#endif  // IROHA_CRYPTO_HPP
