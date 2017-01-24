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

#include "../../core/service/peer_service.hpp"
#include "../../core/crypto/hash.hpp"
#include "../../core/infra/protobuf/convertor.hpp"
#include "../../core/infra/config/peer_service_with_json.hpp"

template<typename T>
using Transaction = transaction::Transaction<T>;
template<typename T>
using ConsensusEvent = event::ConsensusEvent<T>;
template<typename T>
using Add = command::Add<T>;
template<typename T>
using Transfer = command::Transfer<T>;

void setAwkTimer(int const sleepMillisecs, std::function<void(void)> const &action) {
    std::thread([action, sleepMillisecs]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(sleepMillisecs));
        action();
    }).join();
}

int main(){
    std::string senderPublicKey;

    std::string pubKey = config::PeerServiceConfig::getInstance().getMyPublicKey();

    while(1){
        setAwkTimer(3000, [&](){
            auto event = std::make_unique<ConsensusEvent<Transaction<Add<object::Asset>>>>(
                senderPublicKey.c_str(),
                "domain",
                "Dummy transaction",
                100,
                0
            );
            std::cout <<" created event\n";
            event->addTxSignature(
                    config::PeerServiceConfig::getInstance().getMyPublicKey(),
                    signature::sign(event->getHash(),
                                    config::PeerServiceConfig::getInstance().getMyPublicKey(),
                                    config::PeerServiceConfig::getInstance().getPrivateKey()).c_str()
            );
        });
    }

    return 0;
}
