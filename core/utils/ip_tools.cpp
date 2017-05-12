/*
Copyright Soramitsu Co., Ltd. 2016 All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include <utils/ip_tools.hpp>
#include <utils/logger.hpp>

#include <iostream>
#include <regex>

namespace ip_tools {

bool isIpValid(const std::string &ip) {
  std::regex ipRegex(
      "((([0-1]?\\d\\d?)|((2[0-4]\\d)|(25[0-5]))).){3}(([0-1]?\\d\\d?)|((2[0-4]"
      "\\d)|(25[0-5])))");
  return std::regex_match(ip, ipRegex);
}

uint32_t stringIpToUint(const std::string &ip) {
  // ip should be already validated with isIpValid() function
  std::istringstream delimMe(ip);
  std::vector<std::string> dividedIp;
  std::string s;
  while (std::getline(delimMe, s, '.')) {
    dividedIp.push_back(s);
  }

  std::vector<uint8_t> octets;
  octets.push_back((uint8_t)std::stoi(dividedIp[0]));
  octets.push_back((uint8_t)std::stoi(dividedIp[1]));
  octets.push_back((uint8_t)std::stoi(dividedIp[2]));
  octets.push_back((uint8_t)std::stoi(dividedIp[3]));

  uint32_t uintIp =
      (octets[0] << 24) + (octets[1] << 16) + (octets[2] << 8) + octets[3];
  return uintIp;
}

std::string uintIpToString(uint32_t ip) {
  uint8_t o1 = uint8_t(ip & 0xFF);
  ip >>= 8;
  uint8_t o2 = uint8_t(ip & 0xFF);
  ip >>= 8;
  uint8_t o3 = uint8_t(ip & 0xFF);
  ip >>= 8;
  uint8_t o4 = uint8_t(ip & 0xFF);
  std::string result = std::to_string(o4);
  result += ".";
  result += std::to_string(o3);
  result += ".";
  result += std::to_string(o2);
  result += ".";
  result += std::to_string(o1);
  return result;
}

std::pair<uint32_t, uint32_t> getIpRangeByNetmask(const std::string &netmask) {
  std::pair<uint32_t, uint32_t> result;
  std::istringstream delimMe(netmask);
  std::vector<std::string> dividedIp;
  std::string s;
  while (std::getline(delimMe, s, '/')) {
    dividedIp.push_back(s);
  }

  if (dividedIp.size() != 2) {
    return result;
  }

  if (!isIpValid(dividedIp[0])) {
    return result;
  }

  uint32_t cidrmask = (uint32_t)std::stoul(dividedIp[1]);
  if (cidrmask < 16) {
    logger::warning("Networks larger than /16 are not supported for now");
    return result;
  }

  cidrmask = 32 - cidrmask;
  uint32_t bitmask = 0;
  for (uint32_t i = 0; i < cidrmask; ++i) {
    bitmask |= (1u << i);
  }
  bitmask = ~bitmask;
  uint32_t uintIp = stringIpToUint(dividedIp[0]);

  result.first = (uintIp & bitmask) + 1;
  result.second = (1u << cidrmask) - 2;

  return result;
};

}  // namespace ip_tools
