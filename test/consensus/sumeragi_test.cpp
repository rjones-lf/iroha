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

#include "../../core/service/peer_service.hpp"
#include "../../core/util/logger.hpp"
#include "../../core/crypto/hash.hpp"
#include "../../core/infra/protobuf/convertor.hpp"
#include "../../core/infra/config/peer_service_with_json.hpp"

#include "../../core/model/model.hpp"

template<typename T>
using Transaction = transaction::Transaction<T>;
template<typename T>
using ConsensusEvent = event::ConsensusEvent<T>;
template<typename T>
using Add = command::Add<T>;
template<typename T>
using Transfer = command::Transfer<T>;
template<typename T>
using Update = command::Update<T>;

void setAwkTimer(int const sleepMillisecs, const std::function<void(void)>& action) {
    std::thread([action, sleepMillisecs]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(sleepMillisecs));
        action();
    }).join();
}

int main(int argc, char *argv[]){

    try {
        std::string value;
        std::string senderPublicKey;
        std::string receiverPublicKey;
        std::string cmd;
        std::vector <std::unique_ptr<object::Peer>> nodes = config::PeerServiceConfig::getInstance().getPeerList();

        connection::initialize_peer();

        logger::setLogLevel(logger::LogLevel::DEBUG);

        for (const auto &n : nodes) {
            std::cout << "=========" << std::endl;
            std::cout << n->getPublicKey() << std::endl;
            std::cout << n->getIP() << std::endl;
            connection::addSubscriber(n->getIP());
        }

        std::string pubKey = config::PeerServiceConfig::getInstance().getMyPublicKey();

        sumeragi::initializeSumeragi(pubKey, std::move(nodes));

        std::thread connection_th([]() {
            connection::run();
        });

        // since we have thread pool, it sets all necessary callbacks in 
        // sumeragi::initializeSumeragi.
        // std::thread http_th([]() {
        //     sumeragi::loop();
        // });

        if (argc >= 2 && std::string(argv[1]) == "public") {
            while (1) {
                setAwkTimer(10, [&]() {
                    auto event = std::make_unique<ConsensusEvent<Transaction<Update<object::Asset>>>>(
                            config::PeerServiceConfig::getInstance().getMyPublicKey(),
                            config::PeerServiceConfig::getInstance().getMyPublicKey(),
                            "AssetName",
                            100
                    );
                    event->addTxSignature(
                            config::PeerServiceConfig::getInstance().getMyPublicKey(),
                            signature::sign(event->getHash(),
                                            config::PeerServiceConfig::getInstance().getMyPublicKey(),
                                            config::PeerServiceConfig::getInstance().getPrivateKey()).c_str()
                    );
                    connection::send(config::PeerServiceConfig::getInstance().getMyIp(), convertor::encode(*event));
                });
            }
        } else {
            std::cout << "I'm only node\n";
            while (1);
        }
        // http_th.detach();
        connection_th.detach();
    }catch(char const *e){
        std::cout << e << std::endl;
    }

    return 0;
}
