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

#include <iostream>
#include <sstream>

#include "validators.hpp"

namespace iroha_cli {

  bool validate_port(const char *, gflags::int32 port) {
    // TODO 13/10/2017 neewy: Use iroha::network::util::is_port_valid IR-509
    // #goodfirstissue
    if (port > 0 && port < 65535)
      return 1;

    std::cout << "Port can be only in range (0, 65535)\n";
    return 0;
  }

  bool validate_peers(const char *, const std::string &s) {
    std::stringstream ss(s);
    std::string tmp;
    while (std::getline(ss, tmp, ';')) {
      if (tmp.size() != 32) {
        printf("\"%s\" doesn't look like pubkey (size != 32)\n", tmp.c_str());
        return 0;
      }
    }
    return 1;
  }

  bool validate_config(const char *, const std::string &file) {
    if (file.empty()) {
      return false;
    }
    bool valid = true;
    for (const auto &ch : file) {
      valid &= std::isalnum(ch) || ch == '.';
    }
    if (!valid) {
      std::cout << "ERROR: Specify valid config filename.\n";
    }
    return valid;
  }

  bool validate_genesis_block(const char *, const std::string &file) {
    if (file.empty()) {
      return false;
    }
    bool valid = true;
    for (const auto &ch : file) {
      valid &= std::isalnum(ch) || ch == '.';
    }
    if (!valid) {
      std::cout << "ERROR: Specify valid genesis_block json filename.\n";
    }
    return valid;
  }

}  // namespace iroha_cli
