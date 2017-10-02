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

#include <utility>
#include <fstream>

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
  template<typename T, typename V>
  auto deserializeKeypairField(T iroha::keypair_t::*field,
                               const V &value) {
    return [=](auto keypair) mutable {
      return iroha::hexstringToArray<T::size()>(value)
          | iroha::assignObjectField(keypair, field);
    };
  }

  KeysManagerImpl::KeysManagerImpl(std::string account_name)
      : account_name_(std::move(account_name)) {}

  nonstd::optional<iroha::keypair_t> KeysManagerImpl::loadKeys() {
    // Try to load from local file
    std::ifstream priv_file(account_name_ + ".priv");
    std::ifstream pub_file(account_name_ + ".pub");
    if (not priv_file || not pub_file) {
      return nonstd::nullopt;
    }
    std::string client_pub_key_;
    std::string client_priv_key_;
    priv_file >> client_priv_key_;
    pub_file >> client_pub_key_;

    return nonstd::make_optional<iroha::keypair_t>()
        | deserializeKeypairField(&iroha::keypair_t::pubkey,
                                  client_pub_key_)
        | deserializeKeypairField(&iroha::keypair_t::privkey,
                                  client_priv_key_);
  }

  bool KeysManagerImpl::createKeys(std::string pass_phrase) {
    auto seed = iroha::create_seed(pass_phrase);
    auto key_pairs = iroha::create_keypair(seed);
    std::ifstream pb_file(account_name_ + ".pub");
    std::ifstream pr_file(account_name_ + ".priv");
    if (pb_file && pr_file) {
      return false;
    }
    // Save pubkey to file
    std::ofstream pub_file(account_name_ + ".pub");
    if (not pub_file) {
      return false;
    }
    pub_file << key_pairs.pubkey.to_hexstring();
    // Save privkey to file
    std::ofstream priv_file(account_name_ + ".priv");
    if (not priv_file) {
      return false;
    }
    priv_file << key_pairs.privkey.to_hexstring();
    return true;
  }

}  // namespace iroha
