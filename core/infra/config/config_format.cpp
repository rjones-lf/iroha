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

#include "config_format.hpp"
#include "utils/ip_tools.hpp"

namespace config {

using ConfigFormat = config::ConfigFormat;
using nlohmann::json;

ConfigFormat::ConfigFormat() {}

ConfigFormat& ConfigFormat::getInstance() {
  static ConfigFormat instance;
  return instance;
}

bool ConfigFormat::ensureFormatSumeragi(const std::string& configStr) {
  return ensureFormat(configStr, R"(
    {
      "me":{
        "ip":"ip",
        "name":"*",
        "publicKey":"publicKey",
        "privateKey":"privateKey"
      },
      "group":[
        {
          "ip":"ip",
          "name":"*",
          "publicKey":"publicKey"
        }
      ]
    }
  )");
}

bool ConfigFormat::ensureFormat(const std::string& configStr,
                                const std::string& formatConfigStr) {
  json config, formatConfig;

  try {
    config = json::parse(configStr);
  } catch (std::exception& e) {
    return false;
  }

  try {
    formatConfig = json::parse(formatConfigStr);
  } catch (std::exception& e) {
    throw std::domain_error("cannot parse config format. " +
                            std::string(e.what()));
  }

  return ensureFormat(config, formatConfig, "(root)");
}

bool ConfigFormat::ensureFormat(json& actualConfig, json& formatConfig,
                                const std::string& history) {
  if (actualConfig.type() != formatConfig.type()) {
    logger::warning("peer service with json")
        << "type must be " << static_cast<int>(formatConfig.type())
        << ", but is " << static_cast<int>(actualConfig.type());
    return false;
  } else {
    if (actualConfig.is_object()) {
      for (auto it = formatConfig.begin(); it != formatConfig.end(); it++) {
        if (actualConfig.find(it.key()) == actualConfig.end()) {
          logger::warning("peer service with json")
              << "Not found: \"" << it.key() << "\" in " << history;
          return false;
        }
      }

      std::vector<json::iterator> configIters;

      bool res = true;

      for (auto it = actualConfig.begin(); it != actualConfig.end(); it++) {
        if (formatConfig.find(it.key()) == formatConfig.end()) {
          logger::warning("peer service with json")
              << "Unused keys: \"" << it.key() << "\" in " << history;
          return false;
        } else {
          res = res &&
                ensureFormat(it.value(), formatConfig.find(it.key()).value(),
                             history + " : \"" + it.key() + "\"");
        }
      }

      return res;
    } else if (actualConfig.is_string()) {
      const auto& format = formatConfig.get<std::string>();
      const auto& value = actualConfig.get<std::string>();

      if (format == "ip") {
        // cannot use config file because nlohmann::json cannot parse string
        // std::regex ipRegex(FormatRegExConfig::getInstance().getIPRegEx());
        std::regex ipRegex(
            "^(([1-9]?[0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5]).){3}([1-9]?[0-9]|1["
            "0-9]{2}|2[0-4][0-9]|25[0-5])$");
        if (!ip_tools::isIpValid(value)) {
          logger::warning("peer service with json")
              << "IP " << value << " looks like not a valid ip.";
          return false;
        }
        return true;
      } else if (format == "publicKey") {
        // currently do nothing
        return true;
      } else if (format == "privateKey") {
        // currently do nothing
        return true;
      } else if (format == "*") {
        // DO NOTHING. Any string is accepted.
        return true;
      } else {
        throw exception::NotImplementedException("ensureFormat",
                                                 "peer service with json");
      }
    } else if (actualConfig.is_array()) {
      auto formats = formatConfig.get<std::vector<json>>();
      auto values = actualConfig.get<std::vector<json>>();

      if (formats.size() != 1) {
        throw std::domain_error("array size should be 1 , but is " +
                                std::to_string(formats.size()) + ")");
      }

      auto& format = formats.front();

      bool res = true;
      for (auto& value : values) {
        res = res && ensureFormat(value, format, history);
      }
      return res;
    } else {
      throw std::invalid_argument("not implemented error");
    }
  }
}
}  // namespace config