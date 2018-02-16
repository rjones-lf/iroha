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

#include "logger/logger.hpp"

namespace logger {
  const std::string end = "\033[0m";

  std::string red(const std::string &string) {
    const std::string red_start = "\033[31m";
    return red_start + string + end;
  }

  std::string yellow(const std::string &string) {
    const std::string yellow_start = "\033[33m";
    return yellow_start + string + end;
  }

  std::string output(const std::string &string) {
    return yellow("---> " + string);
  }

  std::string input(const std::string &string) {
    return red("<--- " + string);
  }

  static void setGlobalPattern() {
    spdlog::set_pattern("[%H:%M:%S][th: %t][%l] [%n] << %v");
  }

  static std::shared_ptr<spdlog::logger> createLogger(const std::string &tag) {
    setGlobalPattern();
    return spdlog::stdout_color_mt(tag);
  }

  Logger log(const std::string &tag) {
    auto logger = spdlog::get(tag);
    if (logger == nullptr) {
      logger = createLogger(tag);
    }
    return logger;
  }

  Logger testLog(const std::string &tag) {
    return log(tag);
  }

  std::string boolRepr(bool value) {
    return value ? "true" : "false";
  }

}  // namespace logger
