/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef TEST_FUZZING_LOGGER_HPP
#define TEST_FUZZING_LOGGER_HPP

#include "framework/test_logger.hpp"

#include "logger/logger_manager.hpp"

logger::LoggerManagerTreePtr getFuzzLoggerManager() {
  static const std::string kFuzzLogTag{"Fuzz"};
  static logger::LoggerManagerTreePtr log_manager;
  if (!log_manager) {
    log_manager = getTestLoggerManager();
    // fuzzing target is intended to run many times (~millions) so any
    // additional output slows it down significantly
    log_manager->registerChild(
        kFuzzLogTag, logger::LogLevel::kCritical, logger::kDefaultLogPatterns);
  }
  return log_manager->getChild(kFuzzLogTag);
}

logger::LoggerPtr getFuzzLogger(const std::string &tag) {
  return getFuzzLoggerManager()->getChild(tag)->getLogger();
}

#endif // TEST_FUZZING_LOGGER_HPP
