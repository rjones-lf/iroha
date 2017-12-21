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

#include <iostream>
#include <regex>
#include <string>
#include "parser/parser.hpp"

namespace iroha {
  namespace validator {

    /**
     * validates ip v4 address ending with port number
     * @param address
     * @return true if address is valid
     */
    bool isValidIpV4(const std::string &address) {
      std::regex valid_ipv4(
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
     * 4. address may contain single label without dots (i.e. localhost:8080)
     * 5. labels should not have more than 63 characters length
     * @param address
     * @return true if address is valid
     */
    bool isValidHostname(const std::string &address) {
      // get domain and port
      auto domain_and_port = ::parser::split(address, ':');
      // should have exactly two parts: labels and port
      if (domain_and_port.size() != 2) {
        return false;
      }

      auto labels = ::parser::split(domain_and_port[0], '.');
      // should have at least one label
      if (labels.empty()) {
        return false;
      }

      std::regex valid_label_regex(
          R"#((([a-zA-Z0-9]*[a-zA-Z]+[a-zA-Z0-9]+|[a-zA-Z0-9][a-zA-Z0-9\-]*[a-zA-Z0-9])))#");
      // check if all labels are valid using valid_label_regex
      if (not std::all_of(
              labels.begin(), labels.end(), [valid_label_regex](auto label) {
                return label.size() < 64
                    && std::regex_match(label, valid_label_regex);
              })) {
        return false;
      }
      std::cout << "here" << std::endl;
      // get port
      auto port = domain_and_port[1];

      std::regex valid_port_regex(
          R"#(^(6553[0-5]|655[0-2]\d|65[0-4]\d\d|6[0-4]\d{3}|[1-5]\d{4}|[1-9]\d{0,3}|0)$)#");
      return std::regex_match(port, valid_port_regex);
    }

  }  // namespace validator
}  // namespace iroha

#endif  // IROHA_ADDRESS_VALIDATOR_HPP
