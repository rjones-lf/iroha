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

#include <csignal>
#include <atomic>
#include <thread>

#include <service/connection.hpp>
#include <consensus/sumeragi.hpp>
#include <infra/config/peer_service_with_json.hpp>
#include <utils/logger.hpp>
#include <ametsuchi/repository.hpp>

std::atomic_bool running(true);


void signalHandler(int param) {;
  logger::info("main") << "will halt (" << param << ")";
  exit(0);
}

int main() {
  if(std::signal(SIGINT, signalHandler) == SIG_ERR){
    logger::error("main") << "'SIGINT' Signal setting error!";
  }

  if (getenv("IROHA_HOME") == nullptr) {
    logger::error("main") << "You must set IROHA_HOME!";
    return 1;
  }

  logger::setLogLevel(logger::LogLevel::Debug);

  connection::initialize_peer();
  repository::front_repository::initialize_repository();
  sumeragi::initializeSumeragi();
  // peer::izanami::startIzanami();

  std::thread check_server([&](){
      std::string cmd;
      while (running){
          std::cin >> cmd;
          if(cmd == "quit"){
              logger::info("main") << "will halt ";
              connection::finish();
              return;
          }
      }
      logger::info("main") << "OWARI";
  });
  connection::run();
  logger::info("main") << "check_server.detach()";
  running = false;
  check_server.join();
  logger::info("main") << "Finish";
  return 0;
}
