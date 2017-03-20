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

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <thread>

#include <consensus/sumeragi.hpp>
#include <consensus/connection/connection.hpp>

#include <service/peer_service.hpp>
#include <crypto/hash.hpp>
#include <infra/config/peer_service_with_json.hpp>

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
        });
    }

    return 0;
}
