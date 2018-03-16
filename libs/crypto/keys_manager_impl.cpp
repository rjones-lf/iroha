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

#include "crypto/keys_manager_impl.hpp"

#include <spdlog/spdlog.h>
#include <algorithm>
#include <fstream>
#include <random>
#include <utility>

#include "common/byteutils.hpp"
#include "common/types.hpp"  // for keypair_t, pubkey_t, privkey_t
#include "cryptography/ed25519_sha3_impl/internal/ed25519_impl.hpp"
#include "cryptography/ed25519_sha3_impl/internal/sha3_hash.hpp"

using iroha::operator|;

namespace iroha {
  /**
   * Return a function which will try deserialize the value to
   * specified field in given keypair
   * @tparam T - keypair field type
   * @tparam V - value type to deserialize
   * @param field - keypair field to be deserialized
   * @param value - value to be deserialized
   * @return function that will return keypair on success, otherwise nullopt
   */
  template <typename T, typename V>
  static auto deserializeKeypairField(T keypair_t::*field, const V &value) {
    return [=](auto keypair) mutable {
      return hexstringToArray<T::size()>(value)
          | assignObjectField(keypair, field);
    };
  }

  /**
   * Function for the key encryption via XOR
   * @tparam is a key type
   * @param privkey is a private key
   * @param pass_phrase is a key for encryption
   * @return encrypted string
   */
  template <typename T>
  static std::string encrypt(const T &key, const std::string &pass_phrase) {
    std::string ciphertext(key.size(), '\0');
    const auto pass_size = std::max(1ul, pass_phrase.size());

    for (auto i = 0u; i < key.size(); i++) {
      ciphertext[i] = key[i] ^ pass_phrase[i % pass_size];
    }
    return ciphertext;
  }

  /**
   * Function for XOR decryption
   */
  static constexpr auto decrypt = encrypt<std::string>;

  /**
   * Return a function which will try to deserialize and then decrypt private
   * key via XORing with pass phrase
   * @param s is an encrypted data from file
   * @param pass_phrase for decryption
   * @return function that will set keypair::privkey on successful
   *         deserialization and decryption
   */
  static auto deserializedEncrypted(const std::string &s,
                                    const std::string &pass_phrase) {
    constexpr auto size = privkey_t::size();

    return [=](auto keypair) mutable {
      return hexstringToBytestring(s) |
          [&](auto binstr) {
            return boost::make_optional(decrypt(binstr, pass_phrase));
          }
      | stringToBlob<size> | assignObjectField(keypair, &keypair_t::privkey);
    };
  }

  KeysManagerImpl::KeysManagerImpl(const std::string &account_name)
      : account_name_(std::move(account_name)),
        log_(logger::log("KeysManagerImpl")) {}

  bool KeysManagerImpl::validate(const keypair_t &keypair) const {
    std::string test = "12345";
    auto sig =
        sign(sha3_256(test).to_string(), keypair.pubkey, keypair.privkey);
    if (not verify(sha3_256(test).to_string(), keypair.pubkey, sig)) {
      log_->error("key validation failed");
      return false;
    }
    return true;
  }

  bool KeysManagerImpl::loadFile(const std::string &filename,
                                 std::string &res) {
    std::ifstream file(filename);
    if (not file) {
      log_->error("Cannot read '" + filename + "'");
      return false;
    }
    file >> res;
    return true;
  }

  boost::optional<keypair_t> KeysManagerImpl::loadKeys() {
    std::string pub_key;
    std::string priv_key;

    if (not loadFile(account_name_ + kPubExt, pub_key)
        or not loadFile(account_name_ + kPrivExt, priv_key))
      return boost::none;

    return boost::make_optional(keypair_t())
        | deserializeKeypairField(&keypair_t::pubkey, pub_key)
        | deserializeKeypairField(&keypair_t::privkey, priv_key) |
        [this](auto keypair) {
          return this->validate(keypair) ? boost::make_optional(keypair)
                                         : boost::none;
        };
  }

  boost::optional<keypair_t> KeysManagerImpl::loadKeys(
      const std::string &pass_phrase) {
    std::string pub_key;
    std::string priv_key;

    if (not loadFile(account_name_ + kPubExt, pub_key)
        or not loadFile(account_name_ + kPrivExt, priv_key))
      return boost::none;

    return boost::make_optional(keypair_t())
        | deserializeKeypairField(&keypair_t::pubkey, pub_key)
        | deserializedEncrypted(priv_key, pass_phrase) | [this](auto keypair) {
            return this->validate(keypair) ? boost::make_optional(keypair)
                                           : boost::none;
          };
  }

  bool KeysManagerImpl::createKeys() {
    auto key_pairs = create_keypair();

    auto pub = key_pairs.pubkey.to_hexstring();
    auto priv = key_pairs.privkey.to_hexstring();
    return store(pub, priv);
  }

  bool KeysManagerImpl::createKeys(const std::string &pass_phrase) {
    auto key_pairs = create_keypair();

    auto pub = key_pairs.pubkey.to_hexstring();
    auto priv = bytestringToHexstring(encrypt(key_pairs.privkey, pass_phrase));
    return store(pub, priv);
  }

  bool KeysManagerImpl::store(const std::string &pub, const std::string &priv) {
    std::ofstream pub_file(account_name_ + kPubExt);
    std::ofstream priv_file(account_name_ + kPrivExt);
    if (not pub_file or not priv_file) {
      return false;
    }

    pub_file << pub;
    priv_file << priv;
    return pub_file.good() && priv_file.good();
  }

  const std::string KeysManagerImpl::kPubExt = ".pub";
  const std::string KeysManagerImpl::kPrivExt = ".priv";
}  // namespace iroha
