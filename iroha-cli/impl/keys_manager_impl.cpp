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

#include "impl/keys_manager_impl.hpp"
#include <curses.h>
#include <iostream>

#include <fstream>
#include <utility>

using iroha::operator|;

namespace iroha_cli {
  const auto suffix = ".keypair";

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

  KeysManagerImpl::KeysManagerImpl(std::string account_name)
      : account_name_(std::move(account_name)) {}

  nonstd::optional<iroha::keypair_t> KeysManagerImpl::loadKeys() {
    // Try to load from local file

    std::ifstream keyfile(account_name_ + suffix);
    if (not keyfile) {
      return nonstd::nullopt;
    }
    std::string client_pub_key_;
    std::string client_priv_key_;
    keyfile >> client_priv_key_ >> client_pub_key_;

    return nonstd::make_optional<iroha::keypair_t>()
        | deserializeKeypairField(&iroha::keypair_t::pubkey, client_pub_key_)
        | deserializeKeypairField(&iroha::keypair_t::privkey, client_priv_key_);
  }

  /**
   * @brief Prompts for a password.
   * @param prompt
   * @return
   */
  std::string getpass(const char *prompt) {
    printw(prompt);
    noecho();  // disable character echoing

    char buff[128];
    getnstr(buff, sizeof(buff));

    echo();  // enable character echoing again
    return buff;
  }

  bool KeysManagerImpl::createKeys() {
    std::string passphrase;
    initscr();  // enable ncurses
    std::string p1 = getpass("Enter passphrase: ");
    std::string p2 = getpass("Confirm passphrase: ");
    if (p1 == p2) {
      passphrase = p1;
    } else {
      std::cerr << "Entered passphrases are different.";
      exit(1);
    }
    endwin();  // disable ncurses

    auto seed = iroha::create_seed(passphrase);
    auto key_pairs = iroha::create_keypair(seed);

    // check if such keypair exists
    std::ifstream kin(account_name_ + suffix);
    if (kin) {
      std::cerr << "Given keypair exists\n";
      return false;
    }

    // Save to file
    std::ofstream kout(account_name_ + suffix);
    if (not kout) {
      std::cerr << "Can not safe keypair to the file\n";
      return false;
    }

    kout << key_pairs.pubkey.to_hexstring() << std::endl
         << key_pairs.privkey.to_hexstring();

    return true;
  }

}  // namespace iroha_cli
