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

#ifndef __COMMON_CONFIG_HPP_
#define __COMMON_CONFIG_HPP_

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/istreamwrapper.h>

#include <string>

namespace common {

    namespace config {

        using namespace rapidjson;

        class ConfigLoader{
            Document doc;
        public:
            ConfigLoader(const std::string& file);

            int getIntOrElse(const std::string& key, int def);
            std::string getStringOrElse(const std::string& key, std::string def);
            bool getBoolOrElse(const std::string& key, bool def);

        };
    };
};  // namespace common

#endif // __COMMON_CONFIG_HPP_
