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

#include <string>
#include <iostream>
    
#include "datetime.hpp"
#include "logger.hpp"

namespace logger{


    static LogLevel LOG_LEVEL = LogLevel::DEBUG;

    void setLogLevel(LogLevel lv){
        LOG_LEVEL = lv;
    }
    void debug(
        const std::string &name,
        const std::string &message,
        std::ostream &out = std::cout
    ) {
        if(LOG_LEVEL == LogLevel::DEBUG) {
            out << datetime::unixtime_str() << \
              " DEBUG [" << name << "] " << message << std::endl;
        }
    }
    void debug(
        const std::string &name,
        const std::string &message
    ) {
        debug(name, message, std::cout);
    }

    void info(
        const std::string &name,
        const std::string &message,
        std::ostream &out = std::cout
    ) {
        if(static_cast<int>(LOG_LEVEL) <= 1) {
            out << datetime::unixtime_str() << \
            " INFO [" << name << "] " << message << std::endl;
        }
    }
    void info(
        const std::string &name,
        const std::string &message
    ) {
        info(name, message, std::cout);
    }

    void warning(
        const std::string &name,
        const std::string &message,
        std::ostream &out = std::cout
    ) {
        if(static_cast<int>(LOG_LEVEL) <= 2) {
            out << datetime::unixtime_str() << \
            " WARNING [" << name << "] " << message << std::endl;
        }
    }
    void warning(
            const std::string &name,
            const std::string &message
    ) {
        warning(name, message, std::cout);
    }

    void error(
        const std::string &name,
        const std::string &message,
        std::ostream &out = std::cout
    ) {
        if(static_cast<int>(LOG_LEVEL) <= 3) {
            out << datetime::unixtime_str() << \
            " ERROR (-A-) ["<< name << "] "<< message << std::endl;
        }
    }
    void error(
            const std::string &name,
            const std::string &message
    ) {
        error(name, message, std::cout);
    }

    void fital(
        const std::string &name,
        const std::string &message,
        std::ostream &out = std::cout
    ) {
        if(static_cast<int>(LOG_LEVEL) <= 4) {
            out << datetime::unixtime_str() << \
            " FITAL (`o') [" << name << "] " << message << std::endl;
        }
    }
    void fital(
            const std::string &name,
            const std::string &message
    ) {
        fital(name, message, std::cout);
    }

    void explore(
            const std::string &name,
            const std::string &message,
            std::ostream &out = std::cout
    ) {
        if(static_cast<int>(LOG_LEVEL) <= 5) {
            out << datetime::unixtime_str() << \
            "[" << name << "] " << message << std::endl;
        }
    }
    void explore(
            const std::string &name,
            const std::string &message
    ) {
        explore(name, message, std::cout);
    }

};