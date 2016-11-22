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

std::atomic_bool running(true); 

void server(){
  http::server();
}

void sigIntHandler(int param){
  running = false;
  logger::info("main", "will halt");
}

int main() {
  signal(SIGINT, sigIntHandler);

  if(getenv("IROHA_HOME") == nullptr){
    logger::error("main", "You must set IROHA_HOME!");
    return 1;
  }

  logger::error("main","process is :"+std::to_string(getpid()));

  connection::initialize_peer(nullptr);
  sumeragi::initializeSumeragi( peer::getMyPublicKey(), peer::getPeerList());

  std::thread sumeragi_thread(sumeragi::loop);
  std::thread http_thread(http::server);

  connection::exec_subscription(peer::getMyIp());

  while(running);
  
  sumeragi_thread.detach();
  http_thread.detach();

  return 0;
}
