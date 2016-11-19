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
#include "../../core/model/commands/add.hpp"

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

int main(){
    std::string value;
    std::string senderPublicKey;
    std::string receiverPublicKey;
    std::string cmd;
    std::string pubKey = peer::getMyPublicKey();

    while(1){
        std::cout <<"name  in >> ";
        std::cin>> cmd;
        if(cmd == "quit") break;

        if(cmd == "transfer") {
            std::cout <<"(transfer) in >> ";
            std::cin >> value;

            auto event = std::make_unique<ConsensusEvent<Transaction<Transfer<object::Asset>>>>(
                senderPublicKey,
                receiverPublicKey,
                value,
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
        }else if(cmd == "add"){
            std::cout <<"(add) in >> ";
            std::cin >> value;

            auto event = std::make_unique<ConsensusEvent<Transaction<Add<object::Asset>>>>(
                senderPublicKey,
                "mizuki",
                value,
                100,
                100
            );
            std::cout <<" created event\n";
            event->addTxSignature(
                    peer::getMyPublicKey(),
                    signature::sign(event->getHash(), peer::getMyPublicKey(), peer::getPrivateKey()).c_str()
            );
            auto text = json_parse_with_json_nlohman::parser::dump(event->dump());
            std::cout << text << std::endl;
            auto ex = json_parse_with_json_nlohman::parser::load<ConsensusEvent<Transaction<Add<object::Asset>>>>(text);
            std::cout << json_parse_with_json_nlohman::parser::dump(ex->dump()) << std::endl;
        }
    }

    return 0;
}
