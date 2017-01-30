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

#ifndef PEER_SERVICE_WITH_JSON_HPP
#define PEER_SERVICE_WITH_JSON_HPP

#include "../../model/model.hpp"
#include "iroha_config.hpp"
#include <vector>
#include <memory>
#include <set>

namespace config {

    class PeerServiceConfig: IConfig {
    private:

        PeerServiceConfig();
        PeerServiceConfig(const PeerServiceConfig&);
        PeerServiceConfig& operator=(const PeerServiceConfig&);

    public:
        static std::set<object::Peer> peerList;
        static PeerServiceConfig &getInstance();

        std::string getMyPublicKey();
        std::string getPrivateKey();
        std::string getMyIp();
        std::vector<std::unique_ptr<object::Peer>> getPeerList();

        void addPeer( object::Peer );
        void removePeer( object::Peer );

        virtual std::string getConfigName();
    };
}

#endif // PEER_SERVICE_WITH_JSON_HPP
