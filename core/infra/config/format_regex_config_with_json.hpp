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

#ifndef FORMAT_REGEX_CONFIG_WITH_JSON_HPP
#define FORMAT_REGEX_CONFIG_WITH_JSON_HPP

#include "../../service/peer_service.hpp"
#include "iroha_config.hpp"
#include <vector>

namespace config {
    
    namespace formatRegExConfig {
        static const std::string publicKey  = "publicKey";
        static const std::string privateKey = "privateKey";
        static const std::string ip         = "ip";
        static const std::string name       = "name";
        static const std::string group      = "group";
        static const std::string me         = "me";
    }

    class FormatRegExConfig: IConfig {
    private:
        FormatRegExConfig();
        FormatRegExConfig(const FormatRegExConfig&);
        FormatRegExConfig& operator=(const FormatRegExConfig&);

    public:
        static FormatRegExConfig &getInstance() {
            static FormatRegExConfig instance;
            return instance;
        }

        std::string getIPRegEx();
        std::string getConfigName();
    };
}

#endif // FORMAT_REGEX_CONFIG_WITH_JSON_HPP
