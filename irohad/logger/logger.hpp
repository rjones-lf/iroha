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
#ifndef __IROHA_LOGGER_LOGGER_HPP__
#define __IROHA_LOGGER_LOGGER_HPP__

#include <sstream>
#include <string>

#include <spdlog/spdlog.h>

namespace logger {

struct Logger {
    std::shared_ptr<spdlog::logger> console;

    Logger(std::string &&name):
            console(spdlog::stdout_color_mt(name))
    {};

    Logger(const std::string &name):
            console(spdlog::stdout_color_mt(name))
    {};

    template <typename ... Args>
    void debug(const char* fmt, const Args&... args){
        console->debug(fmt,args...);
    }
    template <typename ... Args>
    void debug(const std::string& fmt, const Args&... args){
        console->debug(fmt,args...);
    }

    template <typename ... Args>
    void info(const char* fmt, const Args&... args){
        console->info(fmt,args...);
    }
    template <typename ... Args>
    void info(const std::string& fmt, const Args&... args){
        console->info(fmt,args...);
    }

    template <typename ... Args>
    void warning(const char* fmt, const Args&... args){
        console->warn(fmt,args...);
    }
    template <typename ... Args>
    void warning(const std::string& fmt, const Args&... args){
        console->warn(fmt,args...);
    }

    template <typename ... Args>
    void error(const char* fmt, const Args&... args){
        console->error(fmt,args...);
    }
    template <typename ... Args>
    void error(const std::string& fmt, const Args&... args){
        console->error(fmt,args...);
    }

};

}  // namespace logger

#include "logger.cpp"

#endif
