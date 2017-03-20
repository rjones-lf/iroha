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
#include <util/logger.hpp>
#include <crypto/hash.hpp>
#include <infra/config/peer_service_with_json.hpp>
#include<service/peer_service.hpp>



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

        connection::initialize_peer();

        logger::setLogLevel(logger::LogLevel::Debug);

        sumeragi::initializeSumeragi();

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
