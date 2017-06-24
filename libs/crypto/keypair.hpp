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

#ifndef IROHA_KEYPAIR_H
#define IROHA_KEYPAIR_H

#include <algorithm>
#include <array>
#include <cstdint>
#include <nonstd/optional.hpp>
#include <string>
#include <vector>
#include "common.hpp"
#include "ed25519.h"

namespace iroha {
  namespace crypto {
    /**
     * Represents a keypair: public and private key.
     */
    class Keypair {
     public:
      using signature_t = std::array<uint8_t, ed25519::SIGNATURELEN>;
      using pubkey_t = std::array<uint8_t, ed25519::PUBLEN>;
      using privkey_t = std::array<uint8_t, ed25519::PRIVLEN>;

      //  using signature_t = std::basic_string<uint8_t>;
      //  using pubkey_t = std::basic_string<uint8_t>;
      //  using privkey_t = std::basic_string<uint8_t>;

      /**
       * Build a keypair with public and private key in binary format
       * @param pub
       * @param priv
       */
      explicit Keypair(const pubkey_t &pub, const privkey_t &priv)
          : pubkey(std::move(pub)),
            privkey(std::move(priv)),
            has_private(true) {}

      /**
       * Build a keypair with public and private key in binary format
       * @param pub
       * @param priv
       */
      explicit Keypair(const std::vector<uint8_t> &pub,
                       const std::vector<uint8_t> &priv)
          : has_private(true) {
        std::copy(&pub[0], &pub[ed25519::PUBLEN], &pubkey[0]);
        std::copy(&pub[0], &pub[ed25519::PUBLEN], &privkey[0]);
      }

      /**
       * Build a keypair with public key in binary format
       * @param pub
       */
      explicit Keypair(const std::vector<uint8_t> &pub) : has_private(false) {
        std::copy(&pub[0], &pub[ed25519::PUBLEN], &pubkey[0]);
      }

      /**
       * Build a keypair with public and private key in binary format
       * @param pub
       * @param priv
       */
      explicit Keypair(const uint8_t *pub, const uint8_t *priv)
          : has_private(true) {
        std::copy(pub, pub + ed25519::PUBLEN, &pubkey[0]);
        std::copy(priv, priv + ed25519::PUBLEN, &privkey[0]);
      }

      /**
       * Build a keypair with public key in binary format
       * @param pub
       */
      explicit Keypair(const uint8_t *pub) : has_private(false) {
        std::copy(pub, pub + ed25519::PUBLEN, &pubkey[0]);
      }

      /**
       * Build a keypair with public and private key in binary format (string)
       * @param pub
       * @param priv
       */
      explicit Keypair(const std::string &pub, const std::string &priv)
          : has_private(true) {
        std::copy(&pub[0], &pub[ed25519::PUBLEN], &pubkey[0]);
        std::copy(&priv[0], &priv[ed25519::PUBLEN], &privkey[0]);
      }

      /**
       * Build a keypair with public key in binary format
       * @param pub
       */
      explicit Keypair(const std::string &pub) : has_private(false) {
        std::copy(&pub[0], &pub[ed25519::PUBLEN], &pubkey[0]);
      }

      /**
       * Build a keypair with only public key in binary format
       * Useful for signature verification.
       * @param pub
       */
      explicit Keypair(const pubkey_t &pub)
          : pubkey(std::move(pub)), has_private(false) {}

      /**
       * Build a keypair with public and private key in base64 encoded format
       * (string)
       * @param pub
       * @param priv
       */
      struct tag_base64_encoded {};
      explicit Keypair(const std::string &pub_base64,
                       const std::string &priv_base64, tag_base64_encoded)
          : has_private(true) {
        auto pub = base64_decode(pub_base64);
        auto priv = base64_decode(priv_base64);
        std::copy(&pub[0], &pub[ed25519::PUBLEN], &pubkey[0]);
        std::copy(&priv[0], &priv[ed25519::PUBLEN], &privkey[0]);
      }

      /**
       * Build a keypair with public key in base64 encoded format (string)
       * @param pub
       */
      explicit Keypair(const std::string &pub_base64, tag_base64_encoded)
          : has_private(false) {
        auto pub = base64_decode(pub_base64);
        std::copy(&pub[0], &pub[ed25519::PUBLEN], &pubkey[0]);
      }

      /**
       * Sign the message
       * @param message - arbitrary blob
       * @return nonstd::nullopt if current keypair has no private key,
       * otherwise returns signature
       */
      nonstd::optional<signature_t> sign(const std::vector<uint8_t> &message) {
        // if keypair has no private key, it is not possible to sign
        if (!has_private) return nonstd::nullopt;

        signature_t sig;
        ed25519_sign(&sig[0], message.data(), message.size(), &pubkey[0],
                     &privkey[0]);
        return nonstd::optional<signature_t>(sig);
      }

      /**
       * Sign the message
       * @param message - arbitrary blob
       * @return nonstd::nullopt if current keypair has no private key,
       * otherwise returns signature
       */
      nonstd::optional<signature_t> sign(const uint8_t *message, size_t len) {
        // if keypair has no private key, it is not possible to sign
        if (!has_private) return nonstd::nullopt;

        signature_t sig;
        ed25519_sign(&sig[0], message, len, &pubkey[0], &privkey[0]);
        return nonstd::optional<signature_t>(sig);
      }

      /**
       * Verify the signature against given message.
       * @param msg
       * @param sig
       * @return true if signature is ok, false otherwise
       */
      bool verify(const std::vector<uint8_t> &msg, const signature_t &sig) {
        return 1 == ed25519_verify(&sig[0], msg.data(), msg.size(), &pubkey[0]);
      }

      /**
       * Verify the signature against given message.
       * @param msg
       * @param sig
       * @return true if signature is ok, false otherwise
       */
      bool verify(const std::string &msg, const signature_t &sig) {
        return 1 == ed25519_verify(
                        &sig[0], reinterpret_cast<const uint8_t *>(msg.c_str()),
                        msg.size(), &pubkey[0]);
      }

      /**
       * Getters for public and private keys in "digest" (binary) format
       * @return
       */
      pubkey_t pub_digest() { return pubkey; }
      nonstd::optional<privkey_t> priv_digest() {
        return has_private ? nonstd::optional<privkey_t>(privkey)
                           : nonstd::nullopt;
      }

      /**
       * Getters for public and private keys in "hexdigest" (hex string) format
       * @return
       */
      std::string pub_hexdigest() {
        return digest_to_hexdigest(&pubkey[0], ed25519::PUBLEN);
      }
      nonstd::optional<std::string> priv_hexdigest() {
        auto r = digest_to_hexdigest(&privkey[0], ed25519::PRIVLEN);
        return has_private ? nonstd::optional<std::string>(r) : nonstd::nullopt;
      }

      /**
       * Getters for public and private keys in "base64" (string) format
       * @return
       */
      std::string pub_base64() { return base64_encode(&pubkey[0], ed25519::PUBLEN); }
      nonstd::optional<std::string> priv_base64() {
        auto r = base64_encode(&privkey[0], ed25519::PRIVLEN);
        return has_private ? nonstd::optional<std::string>(r) : nonstd::nullopt;
      }

      /**
       * Getters for public and private keys in "str" format
       * @return
       */
      std::string pub_str() {
        return std::string{pubkey.begin(), pubkey.end()};
      }
      nonstd::optional<std::string> priv_str() {
        return has_private ? nonstd::optional<std::string>(
                                 std::string{pubkey.begin(), pubkey.end()})
                           : nonstd::nullopt;
      }

      static Keypair generate_keypair() {
        constexpr size_t SEEDLEN = 32;

        Keypair::pubkey_t pub;
        Keypair::privkey_t pri;
        std::array<uint8_t, SEEDLEN> seed;

        // ed25519_create_seed may return 1 in case if it can not open
        // /dev/urandom
        if (ed25519_create_seed(seed.data()) == 1) {
          throw std::runtime_error("can not get seed");
        }

        ed25519_create_keypair(pub.data(), pri.data(), seed.data());

        return Keypair(pub, pri);
      }

     private:
      pubkey_t pubkey;
      privkey_t privkey;

      bool has_private;
    };
  }  // namespace crypto
}  // namespace iroha

#endif  // IROHA_KEYPAIR_H
