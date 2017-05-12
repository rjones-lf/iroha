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

#include "logger.hpp"
#include "datetime.hpp"

namespace logger {

//enum class LogLevel { Debug = 0, Explore, Info, Warning, Error, Fatal };
static const std::string level_names[]{
    "DEBUG", "EXPLORE", "INFO", "WARNING", "ERROR (-A-)", "FATAL (`o')"};

class console_sink : public spdlog::sinks::base_sink<std::mutex> {
  using MyType = console_sink;

 public:
  console_sink() {}
  static std::shared_ptr<MyType> instance() {
    static std::shared_ptr<MyType> instance = std::make_shared<MyType>();
    return instance;
  }

  void _sink_it(const spdlog::details::log_msg &msg) override {
    auto output =
        static_cast<int>(LogLevel::Error) <= msg.level ? stderr : stdout;
    fwrite(msg.formatted.data(), sizeof(char), msg.formatted.size(), output);
    fflush(output);
  }

  void flush() override {
    fflush(stdout);
    fflush(stderr);
  }
};

class console_formatter : public spdlog::formatter {
  using MyType = console_formatter;

 public:
  console_formatter() {}
  static std::shared_ptr<MyType> instance() {
    static std::shared_ptr<MyType> instance = std::make_shared<MyType>();
    return instance;
  }

  void format(spdlog::details::log_msg &msg) override {
    msg.formatted << datetime::unixtime_str()
                  << (msg.level != (spdlog::level::level_enum)LogLevel::Explore
                          ? " " + level_names[msg.level]
                          : "")
                  << " [" << *msg.logger_name << "] " << msg.raw.str();
    // write eol
    msg.formatted.write(spdlog::details::os::eol,
                        spdlog::details::os::eol_size);
  }
};

base::base(std::string &&caller, LogLevel level) noexcept
    : caller(caller),
      uncaught(std::uncaught_exception()),
      level((spdlog::level::level_enum)level) {
  console = spdlog::get(caller);
  if (!console) {
    console =
        std::make_shared<spdlog::logger>(caller, console_sink::instance());
    console->set_level((spdlog::level::level_enum)detail::LOG_LEVEL);
    console->set_formatter(console_formatter::instance());
  }
}

base::base(const std::string &caller, LogLevel level) noexcept
    : caller(caller),
      uncaught(std::uncaught_exception()),
      level((spdlog::level::level_enum)level) {
  console = spdlog::get(caller);
  if (!console) {
    console =
        std::make_shared<spdlog::logger>(caller, console_sink::instance());
    console->set_level((spdlog::level::level_enum)detail::LOG_LEVEL);
    console->set_formatter(console_formatter::instance());
  }
}

base::~base() {
  if (!std::uncaught_exception()) {
    console->log(level, stream.str());
  }
}

debug::debug(std::string &&caller) noexcept : base(caller, LogLevel::Debug) {}
debug::debug(const std::string &caller) noexcept
    : base(caller, LogLevel::Debug) {}
info::info(std::string &&caller) noexcept : base(caller, LogLevel::Info) {}
info::info(const std::string &caller) noexcept : base(caller, LogLevel::Info) {}
warning::warning(std::string &&caller) noexcept
    : base(caller, LogLevel::Warning) {}
warning::warning(const std::string &caller) noexcept
    : base(caller, LogLevel::Warning) {}
error::error(std::string &&caller) noexcept : base(caller, LogLevel::Error) {}
error::error(const std::string &caller) noexcept
    : base(caller, LogLevel::Error) {}
fatal::fatal(std::string &&caller) noexcept : base(caller, LogLevel::Fatal) {}
fatal::fatal(const std::string &caller) noexcept
    : base(caller, LogLevel::Fatal) {}
explore::explore(std::string &&caller) noexcept
    : base(caller, LogLevel::Explore) {}
explore::explore(const std::string &caller) noexcept
    : base(caller, LogLevel::Explore) {}
}  // namespace logger
