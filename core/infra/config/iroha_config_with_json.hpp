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

#ifndef IROHA_CONFIG_WITH_JSON_HPP
#define IROHA_CONFIG_WITH_JSON_HPP

#include "abstract_config_manager.hpp"

namespace config {
class IrohaConfigManager : config::AbstractConfigManager {
 private:
  IrohaConfigManager();

  template <typename T>
  T getParam(const std::string& param, const T& defaultValue);

 public:
  static IrohaConfigManager& getInstance();
  std::string getConfigName();

  std::string getDatabasePath(const std::string& defaultValue);
  std::string getJavaClassPath(const std::string& defaultValue);
  std::string getJavaClassPathLocal(const std::string& defaultValue);
  std::string getJavaLibraryPath(const std::string& defaultValue);
  std::string getJavaLibraryPathLocal(const std::string& defaultValue);

  std::string getJavaPolicyPath(const std::string& defaultValue);
  size_t getConcurrency(size_t defaultValue);
  size_t getMaxFaultyPeers(size_t defaultValue);
  size_t getPoolWorkerQueueSize(size_t defaultValue);
  uint16_t getGrpcPortNumber(uint16_t defaultValue);
  uint16_t getHttpPortNumber(uint16_t defaultValue);
  bool getActiveStart(bool defaultValue);
};
}

#endif  // IROHA_CONFIG_WITH_JSON_HPP
