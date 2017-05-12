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

#include "iroha_config_with_json.hpp"

using IrohaConfigManager = config::IrohaConfigManager;

IrohaConfigManager::IrohaConfigManager() {}

IrohaConfigManager& IrohaConfigManager::getInstance() {
  static IrohaConfigManager instance;
  return instance;
}

std::string IrohaConfigManager::getDatabasePath(
    const std::string& defaultValue) {
  return this->getParam<std::string>({"database_path"}, defaultValue);
}

std::string IrohaConfigManager::getJavaClassPath(
    const std::string& defaultValue) {
  return this->getParam<std::string>({"java_class_path"}, defaultValue);
}

std::string IrohaConfigManager::getJavaClassPathLocal(
    const std::string& defaultValue) {
  return this->getParam<std::string>({"java_class_path_local"}, defaultValue);
}

std::string IrohaConfigManager::getJavaLibraryPath(
    const std::string& defaultValue) {
  return this->getParam<std::string>({"java_library_path"}, defaultValue);
}

std::string IrohaConfigManager::getJavaLibraryPathLocal(
    const std::string& defaultValue) {
  return this->getParam<std::string>({"java_library_path_local"}, defaultValue);
}

std::string IrohaConfigManager::getJavaPolicyPath(
    const std::string& defaultValue) {
  return this->getParam<std::string>({"java_policy_path"}, defaultValue);
}

size_t IrohaConfigManager::getConcurrency(size_t defaultValue) {
  return this->getParam<size_t>({"concurrency"}, defaultValue);
}

size_t IrohaConfigManager::getMaxFaultyPeers(size_t defaultValue) {
  return this->getParam<size_t>({"max_faulty_peers"}, defaultValue);
}

size_t IrohaConfigManager::getPoolWorkerQueueSize(size_t defaultValue) {
  return this->getParam<size_t>({"pool_worker_queue_size"}, defaultValue);
}

uint16_t IrohaConfigManager::getGrpcPortNumber(uint16_t defaultValue) {
  return this->getParam<uint16_t>({"grpc_port"}, defaultValue);
}

uint16_t IrohaConfigManager::getHttpPortNumber(uint16_t defaultValue) {
  return this->getParam<uint16_t>({"http_port"}, defaultValue);
}

bool IrohaConfigManager::getActiveStart(bool defaultValue = false) {
  return this->getParam<bool>({"active_start"}, defaultValue);
}

std::vector<std::string> IrohaConfigManager::getTrustedHosts(
    const std::vector<std::string>& defaultValue) {
  return this->getParam<std::vector<std::string>>({"trusted_hosts"},
                                                  defaultValue);
}