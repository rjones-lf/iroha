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

#ifndef IROHA_CONFIG_H
#define IROHA_CONFIG_H

#include <fstream>  // ifstream, ofstream
#include <util/logger.hpp>
#include <util/exception.hpp>
#include <json.hpp>

namespace config {

  using json = nlohmann::json;

  class AbstractConfigManager {
  private:
    std::string readConfigData(const std::string &pathToJSONFile, const std::string &defaultValue) {
      std::ifstream ifs(pathToJSONFile);
      if (ifs.fail()) {
        return defaultValue;
      }

      std::istreambuf_iterator<char> it(ifs);
      return std::string(it, std::istreambuf_iterator<char>());
    }

    json openConfigData() {

      auto iroha_home = getenv("IROHA_HOME");
      if (iroha_home == nullptr) {
        logger::error("config") << "Set environment variable IROHA_HOME";
        exit(EXIT_FAILURE);
      }

      auto configFolderPath = std::string(iroha_home) + "/";
      auto jsonStr = readConfigData(configFolderPath + this->getConfigName(), "");

      if (jsonStr.empty()) {
        logger::warning("config") << "there is no config '" << getConfigName() << "', we will use default values.";
      } else {
        logger::debug("config") << "load json is " << jsonStr;
        parseConfigDataFromString(std::move(jsonStr));
      }

      return _configData;
    }

  protected:
    virtual void parseConfigDataFromString(std::string &&jsonStr) {
      try {
        _configData = json::parse(std::move(jsonStr));
      } catch (...) {
        throw exception::config::ConfigException("Can't parse json: " + getConfigName());
      }
    }

  public:
    virtual std::string getConfigName() = 0;

    json getConfigData() {
      if (_loaded) {
        // If defaultValue is used, _configData is empty, but _loaded = true. It's cofusing. Any good solution?
        return this->_configData;
      } else {
        _loaded = true;
        return openConfigData();
      }
    }

  protected:
    bool _loaded = false;
    json _configData;

  };
}

#endif  // IROHA_CONFIG_H
