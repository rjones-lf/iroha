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

#include "../../core/consensus/connection/connection.hpp"

#include <string>
#include <iostream>

#include <unordered_map>

#include "../../core/consensus/connection/connection.hpp"
#include "../../core/consensus/consensus_event.hpp"
#include "../../core/model/commands/transfer.hpp"
#include "../../core/model/objects/domain.hpp"
#include "../../core/model/transaction.hpp"
#include "../../core/service/peer_service.hpp"

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

    connection::initialize_peer();

    if(std::string(argv[1]) == "sender"){
        connection::addSubscriber(argv[2]);
        while(1){
            auto event = std::make_unique<ConsensusEvent<Transaction<Transfer<object::Asset>>>>(
                "dummy","dummy","command",100
            );
            event->addTxSignature(
                    peer::getMyPublicKey(),
                    signature::sign(event->getHash(), peer::getMyPublicKey(), peer::getPrivateKey()).c_str()
            );
            connection::sendAll(std::move(event));
        }
    }else if(std::string(argv[1]) == "public"){
        connection::run();
        connection::receive([](std::string from,std::unique_ptr<event::Event> event){
            std::cout <<" receive : !" << "\n";
        });
    }

    while(1){}
    connection::finish();
    return 0;
}
