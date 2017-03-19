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

#include <iostream>

#include "datetime.hpp"
#include "logger.hpp"

/*
namespace logger {

    These macro causes sometimes hard readable compilation error.

    #if __cplusplus <= 201402L
    #define TYPE_UNC_EXC    bool
    #define STD_UNC_EXC     std::uncaught_exception
    #define COND_UNC_EXC    ! STD_UNC_EXC()
    #else
    #define TYPE_UNC_EXC    int
    #define STD_UNC_EXC     std::uncaught_exceptions
    #define COND_UNC_EXC    uncaught >= STD_UNC_EXC()
    #endif

    #define LOGGER_DEF_IMPL(LoggerName, UseLevel, HasPrefix, LogType)               \
    LoggerName::LoggerName(std::string&& caller) noexcept                           \
      : caller(std::move(caller)),                                                  \
        uncaught(STD_UNC_EXC())                                                     \
    {}                                                                              \
    LoggerName::LoggerName(const std::string& caller) noexcept                      \
      : caller(caller),                                                             \
        uncaught(STD_UNC_EXC())                                                     \
    {}                                                                              \
    LoggerName::~LoggerName() {                                                     \
        if  ( COND_UNC_EXC                                                          \
              &&                                                                    \
              static_cast<int>(detail::LOG_LEVEL) <= static_cast<int>(UseLevel)     \
            ) {                                                                     \
            const auto useCErr = static_cast<int>(LogLevel::Error) <= static_cast<int>(UseLevel);   \
            ( useCErr ? std::cerr : std::cout )                                     \
                        << datetime::unixtime_str()                                 \
                        << (HasPrefix ?                                             \
                            std::string(" ") + LogType + " [" + caller + "] "       \
                            :                            "["  + caller + "] "       \
                           )                                                        \
                        << stream.str() << std::endl;                         \
        }                                                                     \
    }

    LOGGER_DEF_IMPL(debug,   LogLevel::Debug,    true,   "DEBUG")
    LOGGER_DEF_IMPL(info,    LogLevel::Info,     true,   "INFO")
    LOGGER_DEF_IMPL(warning, LogLevel::Warning,  true,   "WARNING")
    LOGGER_DEF_IMPL(error,   LogLevel::Error,    true,   "ERROR (-A-)")
    LOGGER_DEF_IMPL(fatal,   LogLevel::Fatal,    true,   "FATAL (`o')")
    LOGGER_DEF_IMPL(explore, LogLevel::Explore,  false,  "(EXPLORE)")
}
*/

namespace logger {

  debug::debug(std::string &&caller) noexcept
      : caller(std::move(caller)), uncaught(std::uncaught_exception()) {}

  debug::debug(const std::string &caller) noexcept
      : caller(caller), uncaught(std::uncaught_exception()) {}

  debug::~debug() {
    if (!std::uncaught_exception() &&
        static_cast<int>(detail::LOG_LEVEL) <=
        static_cast<int>(LogLevel::Debug)) {
      const auto useCErr =
          static_cast<int>(LogLevel::Error) <= static_cast<int>(LogLevel::Debug);
      (useCErr ? std::cerr : std::cout)
          << datetime::unixtime_str()
          << (true ? std::string(" ") + "DEBUG" + " [" + caller + "] "
                   : "[" + caller + "] ")
          << stream.str() << std::endl;
    }
  }

  info::info(std::string &&caller) noexcept
      : caller(std::move(caller)), uncaught(std::uncaught_exception()) {}

  info::info(const std::string &caller) noexcept
      : caller(caller), uncaught(std::uncaught_exception()) {}

  info::~info() {
    if (!std::uncaught_exception() &&
        static_cast<int>(detail::LOG_LEVEL) <= static_cast<int>(LogLevel::Info)) {
      const auto useCErr =
          static_cast<int>(LogLevel::Error) <= static_cast<int>(LogLevel::Info);
      (useCErr ? std::cerr : std::cout)
          << datetime::unixtime_str()
          << (true ? std::string(" ") + "INFO" + " [" + caller + "] "
                   : "[" + caller + "] ")
          << stream.str() << std::endl;
    }
  }

  warning::warning(std::string &&caller) noexcept
      : caller(std::move(caller)), uncaught(std::uncaught_exception()) {}

  warning::warning(const std::string &caller) noexcept
      : caller(caller), uncaught(std::uncaught_exception()) {}

  warning::~warning() {
    if (!std::uncaught_exception() &&
        static_cast<int>(detail::LOG_LEVEL) <=
        static_cast<int>(LogLevel::Warning)) {
      const auto useCErr = static_cast<int>(LogLevel::Error) <=
                           static_cast<int>(LogLevel::Warning);
      (useCErr ? std::cerr : std::cout)
          << datetime::unixtime_str()
          << (true ? std::string(" ") + "WARNING" + " [" + caller + "] "
                   : "[" + caller + "] ")
          << stream.str() << std::endl;
    }
  }

  error::error(std::string &&caller) noexcept
      : caller(std::move(caller)), uncaught(std::uncaught_exception()) {}

  error::error(const std::string &caller) noexcept
      : caller(caller), uncaught(std::uncaught_exception()) {}

  error::~error() {
    if (!std::uncaught_exception() &&
        static_cast<int>(detail::LOG_LEVEL) <=
        static_cast<int>(LogLevel::Error)) {
      const auto useCErr =
          static_cast<int>(LogLevel::Error) <= static_cast<int>(LogLevel::Error);
      (useCErr ? std::cerr : std::cout)
          << datetime::unixtime_str()
          << (true ? std::string(" ") + "ERROR (-A-)" + " [" + caller + "] "
                   : "[" + caller + "] ")
          << stream.str() << std::endl;
    }
  }

  fatal::fatal(std::string &&caller) noexcept
      : caller(std::move(caller)), uncaught(std::uncaught_exception()) {}

  fatal::fatal(const std::string &caller) noexcept
      : caller(caller), uncaught(std::uncaught_exception()) {}

  fatal::~fatal() {
    if (!std::uncaught_exception() &&
        static_cast<int>(detail::LOG_LEVEL) <=
        static_cast<int>(LogLevel::Fatal)) {
      const auto useCErr =
          static_cast<int>(LogLevel::Error) <= static_cast<int>(LogLevel::Fatal);
      (useCErr ? std::cerr : std::cout)
          << datetime::unixtime_str()
          << (true ? std::string(" ") + "FATAL (`o')" + " [" + caller + "] "
                   : "[" + caller + "] ")
          << stream.str() << std::endl;
    }
  }

  explore::explore(std::string &&caller) noexcept
      : caller(std::move(caller)), uncaught(std::uncaught_exception()) {}

  explore::explore(const std::string &caller) noexcept
      : caller(caller), uncaught(std::uncaught_exception()) {}

  explore::~explore() {
    if (!std::uncaught_exception() &&
        static_cast<int>(detail::LOG_LEVEL) <=
        static_cast<int>(LogLevel::Explore)) {
      const auto useCErr = static_cast<int>(LogLevel::Error) <=
                           static_cast<int>(LogLevel::Explore);
      (useCErr ? std::cerr : std::cout)
          << datetime::unixtime_str()
          << (false ? std::string(" ") + "(EXPLORE)" + " [" + caller + "] "
                    : "[" + caller + "] ")
          << stream.str() << std::endl;
    }
  }

}
