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

#include <thread>
#include <signal.h>
#include <unistd.h>
#include <atomic>

#include <json.hpp>

#include "../core/server/http_server.hpp"
#include "../core/consensus/connection/connection.hpp"
#include "../core/consensus/sumeragi.hpp"
#include "../core/util/logger.hpp"

#include "../core/service/peer_service.hpp"
#include "../core/infra/config/peer_service_with_json.hpp"

std::atomic_bool running(true);

template<typename T>
using Transaction = transaction::Transaction<T>;
template<typename T>
using ConsensusEvent = event::ConsensusEvent<T>;
template<typename T>
using Add = command::Add<T>;
template<typename T>
using Transfer = command::Transfer<T>;

void server(){
    http::server();
}

void sigIntHandler(int param){
    running = false;
    logger::info("main") << "will halt";
}

int main() {
    signal(SIGINT, sigIntHandler);

    if (getenv("IROHA_HOME") == nullptr){
      logger::error("main") << "You must set IROHA_HOME!";
      return 1;
    }

    logger::info("main") << "process is :" << getpid();
    logger::setLogLevel(logger::LogLevel::DEBUG);

    std::vector<std::unique_ptr<object::Peer>> nodes = config::PeerServiceConfig::getInstance().getPeerList();
    connection::initialize_peer();
    for (const auto& n : nodes){
        connection::addSubscriber(n->getIP());
    }
  
    sumeragi::initializeSumeragi(
                config::PeerServiceConfig::getInstance().getMyPublicKey(),
                config::PeerServiceConfig::getInstance().getPeerList());

    // since we have thread pool, it sets all necessary callbacks in 
    // sumeragi::initializeSumeragi.
    // std::thread sumeragi_thread(sumeragi::loop);
    std::thread http_thread(server);

    connection::run();

    while(running);

    // sumeragi_thread.detach();
    http_thread.detach();

    return 0;
}
