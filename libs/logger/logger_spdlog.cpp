/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "logger/logger_spdlog.hpp"

#include <mutex>

#define SPDLOG_FMT_EXTERNAL

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <boost/assert.hpp>

static const std::string kDefaultPattern =
    R"([%Y-%m-%d %H:%M:%S.%F] [%L] [%n]: %v)";

static const std::map<logger::LogLevel, const spdlog::level::level_enum>
    kSpdLogLevels = {{logger::LogLevel::kTrace, spdlog::level::trace},
                     {logger::LogLevel::kDebug, spdlog::level::debug},
                     {logger::LogLevel::kInfo, spdlog::level::info},
                     {logger::LogLevel::kWarn, spdlog::level::warn},
                     {logger::LogLevel::kError, spdlog::level::err},
                     {logger::LogLevel::kCritical, spdlog::level::critical}};

namespace logger {

  const LogPatterns kDefaultLogPatterns = ([] {
    LogPatterns p;
    p.setPattern(LogLevel::kTrace,
                 R"([%Y-%m-%d %H:%M:%S.%F] [th:%t] [%5l] [%n]: %v)");
    p.setPattern(LogLevel::kInfo, kDefaultPattern);
    return p;
  })();

  void LogPatterns::setPattern(LogLevel level, std::string pattern) {
    patterns_[level] = pattern;
  }

  std::string LogPatterns::getPattern(LogLevel level) const {
    for (auto it = patterns_.rbegin(); it != patterns_.rend(); ++it) {
      if (it->first <= level) {
        return it->second;
      }
    }
    return kDefaultPattern;
  }

  LoggerSpdlog::LoggerSpdlog(std::string tag, ConstLoggerConfigPtr config)
      : tag_(tag),
        config_(std::move(config)),
        logger_(spdlog::stdout_color_mt(tag)) {
    setupLogger();
  }

  void LoggerSpdlog::setupLogger() {
    const auto it = kSpdLogLevels.find(config_->log_level);
    BOOST_ASSERT_MSG(it != kSpdLogLevels.end(), "Unknown log level!");
    logger_->set_level(it == kSpdLogLevels.end()
                           ? kSpdLogLevels.at(kDefaultLogLevel)
                           : it->second);
    logger_->set_pattern(config_->patterns.getPattern(config_->log_level));
  }

  void LoggerSpdlog::logInternal(Level level, const std::string &s) const {
    const auto it = kSpdLogLevels.find(config_->log_level);
    BOOST_ASSERT_MSG(it != kSpdLogLevels.end(), "Unknown log level!");
    logger_->log(it->second, s);
  }

  bool LoggerSpdlog::shouldLog(Level level) const {
    return config_->log_level <= level;
  }
}  // namespace logger
