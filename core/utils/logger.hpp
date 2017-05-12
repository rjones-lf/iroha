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

#include <sstream>
#include <string>

#include <spdlog/spdlog.h>

namespace logger {

enum class LogLevel { Debug = 0, Explore, Info, Warning, Error, Fatal };

namespace detail {
static LogLevel LOG_LEVEL = LogLevel::Debug;
}

inline void setLogLevel(LogLevel lv) {
  detail::LOG_LEVEL = lv;
  spdlog::set_level((spdlog::level::level_enum)lv);
}

struct base {
  explicit base(std::string &&caller, LogLevel level) noexcept;
  explicit base(const std::string &caller, LogLevel level) noexcept;
  virtual ~base() = 0;
  const std::string caller;
  std::stringstream stream;
  bool uncaught = true;
  std::shared_ptr<spdlog::logger> console;
  spdlog::level::level_enum level;
};

template <typename T>
inline base &operator<<(base &record, T &&t) {
  record.stream << std::forward<T>(t);
  return record;
}

template <typename T>
inline base &operator<<(base &&record, T &&t) {
  return record << std::forward<T>(t);
}

struct debug : public base {
  explicit debug(std::string &&caller) noexcept;
  explicit debug(const std::string &caller) noexcept;
};

struct info : public base {
  explicit info(std::string &&caller) noexcept;
  explicit info(const std::string &caller) noexcept;
};

struct warning : public base {
  explicit warning(std::string &&caller) noexcept;
  explicit warning(const std::string &caller) noexcept;
};

struct error : public base {
  explicit error(std::string &&caller) noexcept;
  explicit error(const std::string &caller) noexcept;
};

struct fatal : public base {
  explicit fatal(std::string &&caller) noexcept;
  explicit fatal(const std::string &caller) noexcept;
};

struct explore : public base {
  explicit explore(std::string &&caller) noexcept;
  explicit explore(const std::string &caller) noexcept;
};
}  // namespace logger

#endif