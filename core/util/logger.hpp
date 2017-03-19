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

#ifndef __LOGGER_HPP_
#define __LOGGER_HPP_

#include <iostream>
#include <sstream>
#include <string>

namespace logger {

/*

    #define LOGGER_DEF(LoggerName, UseLevel, HasPrefix, LogType)                \
    struct LoggerName                                                           \
    {                                                                           \
        explicit LoggerName(std::string&& caller) noexcept;                     \
        explicit LoggerName(const std::string& caller) noexcept;                \
        ~LoggerName();                                                          \
        const std::string   caller;                                             \
        std::stringstream   stream;                                             \
        TYPE_UNC_EXC        uncaught;                                           \
    };                                                                          \
    template <typename T>                                                       \
    inline LoggerName& operator << (LoggerName& record, T&& t) {                \
        record.stream << std::forward<T>(t);                                    \
        return record;                                                          \
    }                                                                           \
    template <typename T>                                                       \
    inline LoggerName& operator << (LoggerName&& record, T&& t) {               \
        return record << std::forward<T>(t);                                    \
    }

    LOGGER_DEF(debug,   LogLevel::Debug,    true,   "DEBUG")
    LOGGER_DEF(info,    LogLevel::Info,     true,   "INFO")
    LOGGER_DEF(warning, LogLevel::Warning,  true,   "WARNING")
    LOGGER_DEF(error,   LogLevel::Error,    true,   "ERROR (-A-)")
    LOGGER_DEF(fatal,   LogLevel::Fatal,    true,   "FATAL (`o')")
    LOGGER_DEF(explore, LogLevel::Explore,  false,  "(EXPLORE)")
    
*/

  enum class LogLevel {
    Debug = 0, Info, Warning, Error, Fatal, Explore
  };

  namespace detail {
    static LogLevel LOG_LEVEL = LogLevel::Debug;
  }

  inline void setLogLevel(LogLevel lv) { detail::LOG_LEVEL = lv; }

  struct debug {
    explicit debug(std::string &&caller) noexcept;

    explicit debug(const std::string &caller) noexcept;

    ~debug();

    const std::string caller;
    std::stringstream stream;
    bool uncaught = true;
  };

  template<typename T>
  inline debug &operator<<(debug &record, T &&t) {
    record.stream << std::forward<T>(t);
    return record;
  }

  template<typename T>
  inline debug &operator<<(debug &&record, T &&t) {
    return record << std::forward<T>(t);
  }

  struct info {
    explicit info(std::string &&caller) noexcept;

    explicit info(const std::string &caller) noexcept;

    ~info();

    const std::string caller;
    std::stringstream stream;
    bool uncaught = true;
  };

  template<typename T>
  inline info &operator<<(info &record, T &&t) {
    record.stream << std::forward<T>(t);
    return record;
  }

  template<typename T>
  inline info &operator<<(info &&record, T &&t) {
    return record << std::forward<T>(t);
  }

  struct warning {
    explicit warning(std::string &&caller) noexcept;

    explicit warning(const std::string &caller) noexcept;

    ~warning();

    const std::string caller;
    std::stringstream stream;
    bool uncaught = true;
  };

  template<typename T>
  inline warning &operator<<(warning &record, T &&t) {
    record.stream << std::forward<T>(t);
    return record;
  }

  template<typename T>
  inline warning &operator<<(warning &&record, T &&t) {
    return record << std::forward<T>(t);
  }

  struct error {
    explicit error(std::string &&caller) noexcept;

    explicit error(const std::string &caller) noexcept;

    ~error();

    const std::string caller;
    std::stringstream stream;
    bool uncaught = true;
  };

  template<typename T>
  inline error &operator<<(error &record, T &&t) {
    record.stream << std::forward<T>(t);
    return record;
  }

  template<typename T>
  inline error &operator<<(error &&record, T &&t) {
    return record << std::forward<T>(t);
  }

  struct fatal {
    explicit fatal(std::string &&caller) noexcept;

    explicit fatal(const std::string &caller) noexcept;

    ~fatal();

    const std::string caller;
    std::stringstream stream;
    bool uncaught = true;
  };

  template<typename T>
  inline fatal &operator<<(fatal &record, T &&t) {
    record.stream << std::forward<T>(t);
    return record;
  }

  template<typename T>
  inline fatal &operator<<(fatal &&record, T &&t) {
    return record << std::forward<T>(t);
  }

  struct explore {
    explicit explore(std::string &&caller) noexcept;

    explicit explore(const std::string &caller) noexcept;

    ~explore();

    const std::string caller;
    std::stringstream stream;
    bool uncaught = true;
  };

  template<typename T>
  inline explore &operator<<(explore &record, T &&t) {
    record.stream << std::forward<T>(t);
    return record;
  }

  template<typename T>
  inline explore &operator<<(explore &&record, T &&t) {
    return record << std::forward<T>(t);
  }
}

#endif