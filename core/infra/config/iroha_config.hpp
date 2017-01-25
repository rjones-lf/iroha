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

#ifndef IROHA_CONFIG_H
#define IROHA_CONFIG_H

#include "../../util/use_optional.hpp"
#include "../../util/logger.hpp"
#include "../../../core/vendor/json/src/json.hpp"
#include <fstream>   // ifstream, ofstream

using json = nlohmann::json;

namespace config {

    class IConfig {
    protected:
        virtual optional<json> openConfig(const std::string &configName) {
            if (_configData) {   // already content loaded
                return _configData;
            }

            const auto PathToIROHA_HOME = [](){
                const auto p = getenv("IROHA_HOME");
                return p == nullptr ? "" : std::string(p);
            }();

            if (PathToIROHA_HOME.empty()) {
                logger::error("peer with json") << "You must set IROHA_HOME!";
                exit(EXIT_FAILURE);
            }

            auto jsonStr = openJSONText(PathToIROHA_HOME + "/" + configName);

            logger::info("peer with json") << "load json is " << jsonStr;

            setConfigData(std::move(jsonStr));

            return _configData;
        }

        virtual std::string openJSONText(const std::string& PathToJSONFile) {
            std::ifstream ifs(PathToJSONFile);
            if (ifs.fail()) {
                logger::error("peer with json") << "Not found: " << PathToJSONFile;
                exit(EXIT_FAILURE);
            }

            std::istreambuf_iterator<char> it(ifs);
            return std::string(it, std::istreambuf_iterator<char>());
        }

        virtual void setConfigData(std::string&& jsonStr) {
            try {
                _configData = json::parse(std::move(jsonStr));
            } catch(...) {
                logger::error("peer with json") << "Bad json!!";
                exit(EXIT_FAILURE);
            }
        }

    public:
        virtual std::string getConfigName() = 0;

    protected:
        optional<json> _configData;
    };    
}

#endif // IROHA_CONFIG_H
