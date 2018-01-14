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

#include "keys_manager_impl.hpp"
#include "cryptography/ed25519_sha3_impl/internal/ed25519_impl.hpp"
#include "cryptography/ed25519_sha3_impl/internal/sha3_hash.hpp"
#include "logger/logger.hpp"

#include <fstream>
#include <utility>

using iroha::operator|;

namespace iroha {
  /**
   * Return function which will try to deserialize specified value to specified
   * field in given keypair
   * @tparam T - keypair field type
   * @tparam V - value type to deserialize
   * @param field - keypair field to be deserialized
   * @param value - value to be deserialized
   * @return keypair on success, otherwise nullopt
   */
  template <typename T, typename V>
  auto deserializeKeypairField(T iroha::keypair_t::*field, const V &value) {
    return [=](auto keypair) mutable {
      return iroha::hexstringToArray<T::size()>(value)
          | iroha::assignObjectField(keypair, field);
    };
  }

  KeysManagerImpl::KeysManagerImpl(const std::string &account_name)
      : account_name_(std::move(account_name)),
        log_(logger::log("KeysManagerImpl")) {}

  bool KeysManagerImpl::validate(const iroha::keypair_t &keypair) const {
    std::string test = "12345";
    auto sig = iroha::sign(
        iroha::sha3_256(test).to_string(), keypair.pubkey, keypair.privkey);
    if (not iroha::verify(
            iroha::sha3_256(test).to_string(), keypair.pubkey, sig)) {
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

  nonstd::optional<iroha::keypair_t> KeysManagerImpl::loadKeys() {
    std::string pub_key;
    std::string priv_key;

    if (!loadFile(account_name_ + ".pub", pub_key)
        || !loadFile(account_name_ + ".priv", priv_key))
      return nonstd::nullopt;

    return nonstd::make_optional<iroha::keypair_t>()
        | deserializeKeypairField(&iroha::keypair_t::pubkey, pub_key)
        | deserializeKeypairField(&iroha::keypair_t::privkey, priv_key) |
        [this](auto keypair) {
          return this->validate(keypair) ? nonstd::make_optional(keypair)
                                         : nonstd::nullopt;
        };
  }

  bool KeysManagerImpl::createKeys(const std::string &pass_phrase) {
    auto seed = iroha::create_seed(pass_phrase);
    auto key_pairs = iroha::create_keypair(seed);

    std::ofstream pub_file(account_name_ + ".pub");
    std::ofstream priv_file(account_name_ + ".priv");
    if (not pub_file or not priv_file) {
      return false;
    }
    pub_file << key_pairs.pubkey.to_hexstring();
    priv_file << key_pairs.privkey.to_hexstring();
    return true;
  }

}  // namespace iroha
