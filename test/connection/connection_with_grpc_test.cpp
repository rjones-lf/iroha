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

#include <unordered_map>

#include "../../core/consensus/connection/connection.hpp"
#include "../../core/consensus/consensus_event.hpp"
#include "../../core/model/commands/transfer.hpp"
#include "../../core/model/objects/domain.hpp"
#include "../../core/model/transaction.hpp"
#include "../../core/service/peer_service.hpp"

#include "../../core/infra/protobuf/convertor.hpp"
#include "../../core/infra/protobuf/event.grpc.pb.h"

#include "../../core/infra/config/peer_service_with_json.hpp"

template<typename T>
using Transaction = transaction::Transaction<T>;
template<typename T>
using ConsensusEvent = event::ConsensusEvent<T>;
template<typename T>
using Add = command::Add<T>;
template<typename T>
using Transfer = command::Transfer<T>;

int main(int argc, char* argv[]){
    if(argc != 3){
        return 1;
    }
    logger::setLogLevel(logger::LogLevel::DEBUG);

    connection::initialize_peer();

    if (std::string(argv[1]) == "sender") {
        logger::debug("main") << "I'm sender.";
        connection::addSubscriber(argv[2]);
        logger::debug("main") << "Add subscribed";
        while(1){
            auto event = ConsensusEvent<Transaction<Add<object::Asset>>>(
                    "sender",
                    "domain",
                    "Dummy transaction",
                    100,
                    0
            );

            logger::debug("main") << "issued event";
            event.addSignature(
                    config::PeerServiceConfig::getInstance().getMyPublicKey(),
                    signature::sign(event.getHash(),
                                    config::PeerServiceConfig::getInstance().getMyPublicKey(),
                                    config::PeerServiceConfig::getInstance().getPrivateKey()).c_str()
            );

            event.addSignature(
                    config::PeerServiceConfig::getInstance().getMyPublicKey(),
                    signature::sign(event.getHash(),
                                    config::PeerServiceConfig::getInstance().getMyPublicKey(),
                                    config::PeerServiceConfig::getInstance().getPrivateKey()).c_str()
            );


            logger::debug("main") << "Add signatured";
            logger::debug("main") << "start send";
            std::cout << " sig:" << event.eventSignatures().size() << "\n";
            connection::sendAll(convertor::encode(event));
        }
    } else if (std::string(argv[1]) == "receive") {
        connection::receive([](const std::string& from,Event::ConsensusEvent& event) {
            std::cout <<" receive : order:" << event.order() << "\n";
            std::cout <<" receive : sig size:" << event.eventsignatures_size() << "\n";
            //std::cout <<" receive : value:" << event.transaction().asset().value() << "\n";
            std::cout <<" receive : name:" << event.transaction().asset().name() <<"\n";
            std::cout <<" type:"<<  event.transaction().type()  << "\n";
        });
        connection::run();
    }

    while(1){}
    connection::finish();
    return 0;
}
