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

#include "../../core/model/commands/add.hpp"
#include "../../core/model/commands/remove.hpp"

#include "../../core/model/objects/peer.hpp"
#include "../../core/infra/config/peer_service_with_json.hpp"
#include "../../core/service/peer_service.hpp"

#include <gtest/gtest.h>
#include <memory>
#include <iostream>

TEST(AddCommand, addpeer){

    std::string ip = "0.0.0.9";
    std::string publicKey = "public_sate";

    command::Add<object::Peer> addPeer( object::Peer( ip, publicKey ) );
    addPeer.execution();
    for( const auto &node : config::PeerServiceConfig::getInstance().getPeerList() ) {
        std::cout << "ip : " << node->getIP() << std::endl;
        std::cout << "address : " << node->getPublicKey() << std::endl;
        std::cout << "trust Rate : " << node->getTrustScore() << std::endl;
    }

    std::cout << std::endl;

    command::Remove<object::Peer> removePeer( object::Peer( ip, publicKey ) );
    removePeer.execution();
    for( const auto &node : config::PeerServiceConfig::getInstance().getPeerList() ) {
        std::cout << "ip : " << node->getIP() << std::endl;
        std::cout << "address : " << node->getPublicKey() << std::endl;
        std::cout << "trust Rate : " << node->getTrustScore() << std::endl;
    }

}
