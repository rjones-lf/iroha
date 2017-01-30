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

    std::set< object::Peer > PeerServiceConfig::peerList;
    PeerServiceConfig::PeerServiceConfig() {
        if( peerList.empty() ) {
            if (auto config = openConfig(getConfigName())) {
                for (const auto& peer : (*config)["group"].get<std::vector<json>>()){
                    peerList.insert( object::Peer(
                        peer["ip"].get<std::string>(),
                        peer["publicKey"].get<std::string>(),
                        1
                    ));
                }
            }
        }
    }

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

    std::vector<std::unique_ptr<object::Peer>> PeerServiceConfig::getPeerList() {
        std::vector<std::unique_ptr<object::Peer>> nodes;
        for( auto node : peerList ) {
            nodes.push_back( std::make_unique<object::Peer>( node.getIP(), node.getPublicKey(), node.getTrustScore() ) );
        }
        sort( nodes.begin(), nodes.end(),
            []( const std::unique_ptr<object::Peer> &a, const std::unique_ptr<object::Peer> &b ) { return a->getTrustScore() > b->getTrustScore(); } );
        return nodes;
    }

    void PeerServiceConfig::addPeer( object::Peer peer ) {
        peerList.insert( peer );
        std::cout << "addPeer: " << peer.getIP() << std::endl;
    }

    void PeerServiceConfig::removePeer( object::Peer peer ) {
        peerList.erase( peerList.find( peer ) );
    }

    std::string PeerServiceConfig::getConfigName() {
        return "config/sumeragi.json";
    }
};
