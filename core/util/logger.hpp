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
#include <sstream>

#include "datetime.hpp"

#if __cplusplus <= 201402L
#define TYPE_UNC_EXC    bool
#define STD_UNC_EXC     std::uncaught_exception
#define COND_UNC_EXC    ! STD_UNC_EXC()
#else
#define TYPE_UNC_EXC    int
#define STD_UNC_EXC     std::uncaught_exceptions
#define COND_UNC_EXC    uncaught >= STD_UNC_EXC()
#endif

namespace logger {

    enum class LogLevel {
        DEBUG = 0,
        INFO,
        WARNING,
        ERROR,
        FATAL,
        EXPLORE
    };

    namespace detail {
        static LogLevel LOG_LEVEL = LogLevel::DEBUG;
    }

    inline void setLogLevel(LogLevel lv){
        detail::LOG_LEVEL = lv;
    }

    // I think this code is ugly.
    #define LOGGER_DEF(LoggerName, UseLevel, HasPrefix, LogType)                        \
    struct LoggerName                                                                   \
    {                                                                                   \
        LoggerName(std::string&& caller) noexcept                                       \
          : caller(std::move(caller)),                                                  \
            uncaught(STD_UNC_EXC())                                                     \
        {}                                                                              \
        ~LoggerName() {                                                                 \
            if  ( COND_UNC_EXC                                                          \
                  &&                                                                    \
                  static_cast<int>(detail::LOG_LEVEL) <= static_cast<int>(UseLevel)     \
                ) {                                                                     \
                const auto useCErr = static_cast<int>(LogLevel::ERROR) <= static_cast<int>(UseLevel);   \
                ( useCErr ? std::cerr : std::cout )                                     \
                            << datetime::unixtime_str()                                 \
                            << (HasPrefix ?                                             \
                                std::string(" ") + LogType + " [" + caller + "] "       \
                                :                            "["  + caller + "] "       \
                               )                                                        \
                            << stream.str() << std::endl;                       \
            }                                                                   \
        }                                                                       \
        const std::string   caller;                                             \
        std::stringstream   stream;                                             \
        TYPE_UNC_EXC        uncaught;                                           \
    };                                                                          \
    template <typename T>                                                       \
    LoggerName& operator << (LoggerName& record, T&& t) {                       \
        record.stream << std::forward<T>(t);                                    \
        return record;                                                          \
    }                                                                           \
    template <typename T>                                                       \
    LoggerName& operator << (LoggerName&& record, T&& t) {                      \
        return record << std::forward<T>(t);                                    \
    }

    LOGGER_DEF(debug,   LogLevel::DEBUG,    true,   "DEBUG")
    LOGGER_DEF(info,    LogLevel::INFO,     true,   "INFO")
    LOGGER_DEF(warning, LogLevel::WARNING,  true,   "WARNING")
    LOGGER_DEF(error,   LogLevel::ERROR,    true,   "ERROR (-A-)")
    LOGGER_DEF(fatal,   LogLevel::FATAL,    true,   "FATAL (`o')")
    LOGGER_DEF(explore, LogLevel::EXPLORE,  false,  "(EXPLORE)")

} // namespace logger
#endif
