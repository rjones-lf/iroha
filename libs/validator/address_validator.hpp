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

#ifndef IROHA_ADDRESS_VALIDATOR_HPP
#define IROHA_ADDRESS_VALIDATOR_HPP

#include <regex>
#include <string>

namespace iroha {
  namespace validator {
    /**
     * validates ip v4 address ending with port number
     * @param address
     * @return true if address is valid
     */
    inline bool isValidIpV4(const std::string &address) {
      static const std::regex valid_ipv4(
          R"#((^((([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\.){3})#"
          R"#(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])):)#"
          R"#((6553[0-5]|655[0-2]\d|65[0-4]\d\d|6[0-4]\d{3}|[1-5]\d{4}|[1-9]\d{0,3}|0)$))#");
      return std::regex_match(address, valid_ipv4);
    }

    /**
     * validates dns hostname address ending with port number
     * Address should:
     * 1. start with label containing at least one letter
     * 2. every label should be split by dots
     * 3. label may contain letters, numbers and dashes
     * 4. address may contain single label without dots (i.e.
     * localhost:8080)
     * 5. labels should not have more than 63 characters length
     * @param address
     * @return true if address is valid
     */
    inline bool isValidHostname(const std::string &address) {
      static const std::regex valid_hostname(
          R"#((([a-zA-Z0-9][a-zA-Z]{1,61}[a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\-]{0,61}[a-zA-Z0-9])\.)*)#"
          R"#(([a-zA-Z0-9][a-zA-Z]{1,61}[a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\-]{0,61}[a-zA-Z0-9]):)#"
          R"#((6553[0-5]|655[0-2]\d|65[0-4]\d\d|6[0-4]\d{3}|[1-5]\d{4}|[1-9]\d{0,3}|0))#");
      return std::regex_match(address, valid_hostname);
    }

  }  // namespace validator
}  // namespace iroha

#endif  // IROHA_ADDRESS_VALIDATOR_HPP
