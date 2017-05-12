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
#include <json.hpp>
#include <utils/expected.hpp>
#include <utils/logger.hpp>
#include "config_utils.hpp"

namespace config {

using json = nlohmann::json;

class AbstractConfigManager {
 private:
  Expected<std::string> readConfigData(const std::string& pathToJSONFile) {
    std::ifstream ifs(pathToJSONFile);
    if (ifs.fail()) {
      return makeUnexpected(exception::NotFoundPathException(pathToJSONFile));
    }

    std::istreambuf_iterator<char> it(ifs);
    return std::string(it, std::istreambuf_iterator<char>());
  }

  json openConfigData() {
    auto res = readConfigData(getConfigPath());
    if (res) {
      logger::debug("config") << "load json is " << *res;
      parseConfigDataFromString(*res);
    } else {
      try {
        std::rethrow_exception(res.excptr());
      } catch (exception::NotFoundPathException& e) {
        logger::warning("config") << res.error();
        logger::warning("config") << "There is no config '" << getConfigPath()
                                  << "', we will use default values.";
      }
    }

    return _configData;
  }

 protected:
  template <typename T>
  T getParam(std::initializer_list<const std::string> params,
             const T& defaultValue) {
    auto tempConfigData = getConfigData();
    try {
      size_t i = 0;
      for (auto& param : params) {
        ++i;
        if (i == params.size()) {
          return tempConfigData.value(param, defaultValue);
        }
        tempConfigData = tempConfigData[param];
      }
      return tempConfigData;
    } catch (...) {
      return defaultValue;
    }
  }

  template <typename T>
  T getParamWithAssert(std::initializer_list<std::string> params) {
    auto tempConfigData = getConfigData();
    auto defaultValue = T();
    try {
      size_t i = 0;
      for (auto& param : params) {
        ++i;
        if (i == params.size()) {
          return tempConfigData.value(param,defaultValue);
        }
        tempConfigData = tempConfigData[param];
      }
      return tempConfigData;
    } catch (...) {
      std::string list_name = "";
      for( auto s : params ) list_name += "\"" + s + "\", ";
      list_name.erase(list_name.end()-2,list_name.end());
      logger::error("config") << "not Found { " << list_name << " } in " << getConfigName();
      assert(false);
    }
  }

  virtual VoidHandler parseConfigDataFromString(const std::string& jsonStr) {
    try {
      _configData = json::parse(std::move(jsonStr));
      return {};
    } catch (...) {
      return makeUnexpected(exception::config::ParseException(getConfigPath()));
    }
  }

 public:
  virtual std::string getConfigName() = 0;
  std::string getConfigPath() { return get_iroha_home() + getConfigName(); }

  json getConfigData() {
    if (_loaded) {
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
}  // namespace config

#endif  // IROHA_CONFIG_H
