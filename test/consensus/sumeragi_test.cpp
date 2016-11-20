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

#include "../../core/consensus/sumeragi.hpp"

#include "../../core/consensus/connection/connection.hpp"
#include "../../core/model/commands/transfer.hpp"
#include "../../core/model/objects/domain.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <thread>

#include "../../core/service/json_parse_with_json_nlohman.hpp"

#include "../../core/service/peer_service.hpp"
#include "../../core/crypto/hash.hpp"

template<typename T>
using Transaction = transaction::Transaction<T>;
template<typename T>
using ConsensusEvent = event::ConsensusEvent<T>;
template<typename T>
using Add = command::Add<T>;
template<typename T>
using Transfer = command::Transfer<T>;

void setAwkTimer(int const sleepMillisecs, std::function<void(void)> const action) {
    std::thread([action, sleepMillisecs]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(sleepMillisecs));
        action();
    }).join();
}

int main(int argc, char *argv[]){
    std::string value;
    std::string senderPublicKey;
    std::string receiverPublicKey;
    std::string cmd;
    std::vector<std::unique_ptr<peer::Node>> nodes = peer::getPeerList();

    connection::initialize_peer(nullptr);

    for(const auto& n : nodes){
        std::cout<< "=========" << std::endl;
        std::cout<< n->getPublicKey() << std::endl;
        std::cout<< n->getIP() << std::endl;
        connection::addPublication(n->getIP());
    }

    std::string pubKey = peer::getMyPublicKey();

    sumeragi::initializeSumeragi( pubKey, std::move(nodes));

    std::thread http_th( []() {
        sumeragi::loop();
    });

    connection::exec_subscription(peer::getMyIp());    
    if( argc >= 2 && std::string(argv[1]) == "public"){
        std::cout<<"start publish tx\n";
        while(1){
            
            setAwkTimer(100, [&](){
                auto event = std::make_unique<ConsensusEvent<Transaction<Transfer<object::Asset>>>>(
                    senderPublicKey,
                    receiverPublicKey,
                    "Dummy transaction",
                    100
                );
                std::cout <<" created event\n";
                event->addTxSignature(
                        peer::getMyPublicKey(),
                        signature::sign(event->getHash(), peer::getMyPublicKey(), peer::getPrivateKey()).c_str()
                );
                auto text = json_parse_with_json_nlohman::parser::dump(event->dump());
                connection::send(peer::getMyIp(), text);
            });
        }
    }else{
        std::cout<<"I'm only node\n";
        while(1);
    }

    http_th.detach();
    return 0;
}
