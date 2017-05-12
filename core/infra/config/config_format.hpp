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

#ifndef CONFIG_FORMAT_HPP
#define CONFIG_FORMAT_HPP

#include <json.hpp>
#include <regex>
#include <utils/exception.hpp>
#include <utils/logger.hpp>

namespace config {
class ConfigFormat {
 public:
  static ConfigFormat& getInstance();
  bool ensureFormatSumeragi(const std::string& configStr);

 private:
  ConfigFormat();
  bool ensureFormat(const std::string& configStr,
                    const std::string& formatConfigStr);
  bool ensureFormat(nlohmann::json& actualConfig, nlohmann::json& formatConfig,
                    const std::string& history);
};
}

#endif
