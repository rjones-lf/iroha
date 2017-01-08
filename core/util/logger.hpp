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

#include <string>
#include <iostream>

#include "datetime.hpp"

namespace logger {

    enum class LogLevel {
        DEBUG = 0,
        INFO,
        WARNING,
        ERROR,
        FITAL,
        EXPLORE
    };

    namespace detail {
        static LogLevel LOG_LEVEL = LogLevel::DEBUG;
    }

    inline void setLogLevel(LogLevel lv) {
        detail::LOG_LEVEL = lv;
    }

    struct __AttachMessage {
        void operator = (std::ostream& ost) {
            ost << std::endl;
        }
    };

}

#define LOG_BASE_PREFIX(logLevel) \
      if (static_cast<int>(logger::detail::LOG_LEVEL) <= logLevel)  \
        logger::__AttachMessage() = std::cout << datetime::unixtime_str()

#define LOG_BASE_MESSAGE(type, where, logLevel) \
      LOG_BASE_PREFIX(logLevel) << " " << type << " [" << where << "] "

#define LOG_DEBUG(where)    LOG_BASE_MESSAGE("DEBUG",       where, 0)
#define LOG_INFO(where)     LOG_BASE_MESSAGE("INFO",        where, 1)
#define LOG_WARNING(where)  LOG_BASE_MESSAGE("WARNING",     where, 2)
#define LOG_ERROR(where)    LOG_BASE_MESSAGE("ERROR (-A-)", where, 3)
#define LOG_FATAL(where)    LOG_BASE_MESSAGE("FATAL (`o')", where, 4)
#define LOG_EXPLORE(where) \
      LOG_BASE_PREFIX(5) << "[" << where << "] "

#endif
