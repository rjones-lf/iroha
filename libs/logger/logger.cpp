/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
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

  static void setGlobalPattern(spdlog::logger &logger) {
    logger.set_pattern("[%Y-%m-%d %H:%M:%S.%F] %n %v");
  }

  static void setDebugPattern(spdlog::logger &logger) {
    logger.set_pattern("[%Y-%m-%d %H:%M:%S.%F][th:%t][%l] %n %v");
  }

  static std::shared_ptr<spdlog::logger> createLogger(const std::string &tag,
                                                      bool debug_mode = true) {
    auto logger = spdlog::stdout_color_mt(tag);
    if (debug_mode) {
      setDebugPattern(*logger);
    } else {
      setGlobalPattern(*logger);
    }
    return logger;
  }

  Logger log(const std::string &tag) {
    static std::mutex mutex;
    std::lock_guard<std::mutex> lock(mutex);
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
