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
<<<<<<< HEAD
#include "../../core/model/commands/add.hpp"
=======
#include "../../core/model/objects/domain.hpp"
>>>>>>> master

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <thread>

<<<<<<< HEAD
#include "../../core/service/json_parse_with_json_nlohman.hpp"
=======
#include "../../core/service/json_parse_with_json_nlohmann.hpp"
>>>>>>> master

#include "../../core/service/peer_service.hpp"
#include "../../core/crypto/hash.hpp"

<<<<<<< HEAD
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

int main(){
    std::string value;
    std::string senderPublicKey;
    std::string receiverPublicKey;
=======
int main(){
>>>>>>> master
    std::string cmd;
    std::string pubKey = peer::getMyPublicKey();

    while(1){
<<<<<<< HEAD
        setAwkTimer(3000, [&](){
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
            std::cout << text << std::endl;
            auto ex = json_parse_with_json_nlohman::parser::load<ConsensusEvent<Transaction<Transfer<object::Asset>>>>(text);
            std::cout << json_parse_with_json_nlohman::parser::dump(ex->dump()) << std::endl;

        });
=======
        std::cout <<"name  in >> ";
        std::cin>> cmd;
        if(cmd == "quit") break;

        auto tx = std::make_unique<transaction::Transaction<command::Transfer<domain::Domain>>>(
                std::make_unique<command::Transfer<domain::Domain>>(
                        std::make_unique<domain::Domain>( peer::getMyPublicKey(), cmd)
                )
        );

        tx->addTxSignature(
                peer::getMyPublicKey(),
                signature::sign(tx->getHash(), peer::getMyPublicKey(), peer::getPrivateKey()).c_str()
        );
        auto event = consensus_event::ConsensusEvent<
                transaction::Transaction<command::Transfer<domain::Domain>>,
                command::Transfer<domain::Domain>
        >(std::move(tx));
        auto parser = json_parse_with_json_nlohman::JsonParse<
                consensus_event::ConsensusEvent<
                        transaction::Transaction<command::Transfer<domain::Domain>>,
                        command::Transfer<domain::Domain>
                >
        >();
        std::cout<<  parser.dump(event.dump()) << std::endl;
>>>>>>> master
    }

    return 0;
}
