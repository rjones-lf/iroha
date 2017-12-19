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

#include "crypto/hash.hpp"
#include "crypto/crypto.hpp"
#include <ed25519/ed25519.h>

namespace iroha {

  /**
   * Sign message with ed25519 crypto algorithm
   * @param msg
   * @param msgsize
   * @param pub
   * @param priv
   * @return
   */
  sig_t sign(const uint8_t *msg,
             size_t msgsize,
             const pubkey_t &pub,
             const privkey_t &priv) {
    sig_t sig;
    ed25519_sign(reinterpret_cast<signature_t *>(&sig),
                 msg,
                 msgsize,
                 reinterpret_cast<const public_key_t *>(pub.data()),
                 reinterpret_cast<const private_key_t *>(priv.data()));
    return sig;
  }

  sig_t sign(const std::string &msg,
             const pubkey_t &pub,
             const privkey_t &priv) {
    return sign(
        reinterpret_cast<const uint8_t *>(msg.data()), msg.size(), pub, priv);
  }

  /**
   * Verify signature of ed25519 crypto algorithm
   * @param msg
   * @param msgsize
   * @param pub
   * @param sig
   * @return true if signature is valid, false otherwise
   */
  bool verify(const uint8_t *msg,
              size_t msgsize,
              const pubkey_t &pub,
              const sig_t &sig) {
    return 0
        != ed25519_verify(reinterpret_cast<const signature_t *>(sig.data()),
                          msg,
                          msgsize,
                          reinterpret_cast<const public_key_t *>(pub.data()));
  }

  bool verify(const std::string &msg, const pubkey_t &pub, const sig_t &sig) {
    return verify(
        reinterpret_cast<const uint8_t *>(msg.data()), msg.size(), pub, sig);
  }

  /**
   * Generate random seed reading from /dev/urandom
   */
  blob_t<32> create_seed(){
    blob_t<32> seed;
    randombytes(seed.data(), seed.size());
    return seed;
  }

  /**
   * Generate random seed as sha3_256(passphrase)
   * @param passphrase
   * @return
   */
  blob_t<32> create_seed(std::string passphrase){
    return sha3_256(passphrase);
  }

  /**
   * Create new keypair
   * @param seed
   * @return
   */
  keypair_t create_keypair(blob_t<32> seed){
    keypair_t kp;
    public_key_t pub{};
    private_key_t priv{};
    ed25519_create_keypair(&priv, &pub);

    std::copy(pub.data, pub.data + ed25519_pubkey_SIZE, kp.pubkey.begin());
    std::copy(priv.data, priv.data + ed25519_privkey_SIZE, kp.privkey.begin());

    return kp;
  }

  /**
   * Create new keypair with a default seed (by create_seed())
   * @return
   */
  keypair_t create_keypair(){
    blob_t<32> seed = create_seed();
    return create_keypair(seed);
  }
}
