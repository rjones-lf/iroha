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

#include "peer_service_with_json.hpp"

namespace config {
    PeerServiceConfig::PeerServiceConfig() {}

    PeerServiceConfig& PeerServiceConfig::getInstance() {
        static PeerServiceConfig serviceConfig;
        return serviceConfig;
    }

    std::string PeerServiceConfig::getMyPublicKey() {
        if (auto config = openConfig(getConfigName())) {
            return (*config)["me"]["publicKey"].get<std::string>();
        }
        return "";
    }

    std::string PeerServiceConfig::getPrivateKey() {
        if (auto config = openConfig(getConfigName())) {
            return (*config)["me"]["privateKey"].get<std::string>();
        }
        return "";
    }

    std::string PeerServiceConfig::getMyIp() {
        if (auto config = openConfig(getConfigName())) {
            return (*config)["me"]["ip"].get<std::string>();
        }
        return "";
    }

    std::vector<std::unique_ptr<peer::Node>> PeerServiceConfig::getPeerList() {
        std::vector<std::unique_ptr<peer::Node>> nodes;
        if (auto config = openConfig(getConfigName())) {
            for (const auto& peer : (*config)["group"].get<std::vector<json>>()){
                nodes.push_back(std::make_unique<peer::Node>(
                    peer["ip"].get<std::string>(),
                    peer["publicKey"].get<std::string>(),
                    1
                ));
            }
        }
        return nodes;
    }

    std::string PeerServiceConfig::getConfigName() {
        return "config/sumeragi.json";
    }
};
