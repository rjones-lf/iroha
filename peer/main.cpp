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

#include <atomic>
#include <signal.h>
#include <thread>
#include <unistd.h>

#include <consensus/connection/connection.hpp>
#include <consensus/sumeragi.hpp>
#include <infra/config/peer_service_with_json.hpp>
#include <repository/world_state_repository.hpp>
#include <server/http_server.hpp>
#include <service/izanami.hpp>
#include <service/peer_service.hpp>
#include <util/logger.hpp>

std::atomic_bool running(true);

void server(){
    http::server();
}

void sigHandler(int param){
    running = false;
    repository::world_state_repository::finish();
    logger::info("main") << "will halt";
}

int main() {
    signal(SIGINT,  sigHandler);
    signal(SIGHUP,  sigHandler);
    signal(SIGTERM, sigHandler);

    if (getenv("IROHA_HOME") == nullptr){
      logger::error("main") << "You must set IROHA_HOME!";
      return 1;
    }

    logger::info("main") << "process is :" << getpid();
    logger::setLogLevel(logger::LogLevel::Debug);

    connection::initialize_peer();
    sumeragi::initializeSumeragi();
    peer::izanami::startIzanami();

    std::thread http_thread(server);

    connection::run();

    while(running);

    // sumeragi_thread.detach();
    http_thread.detach();

    return 0;
}
