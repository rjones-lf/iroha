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

#include <api/command_service.hpp>
#include <api/query_service.hpp>
#include <consensus/connection/service.hpp>
#include <ordering/connection/service.hpp>
#include <logger/logger.hpp>

#include "server_runner.hpp"


int main(int argc,char* argv[]) {
  auto log = logger::Logger("main");

  log.info(R"( dP  888888ba   .88888.  dP     dP   .d888888 )");
  log.info(R"( 88  88    `8b d8'   `8b 88     88  d8'    88 )");
  log.info(R"( 88 a88aaaa8P' 88     88 88aaaaa88a 88aaaaa88a)");
  log.info(R"( 88  88   `8b. 88     88 88     88  88     88 )");
  log.info(R"( 88  88     88 Y8.   .8P 88     88  88     88 )");
  log.info(R"( dP  dP     dP  `8888P'  dP     dP  88     88 )");
  log.info(R"( ---------o=========================o---------)");
  log.info(R"(         = 分散台帳Application いろは =         )");
  log.info(R"( ---------o=========================o---------)");

  connection::api::CommandService commandService;
  connection::api::QueryService queryService;
  connection::consensus::SumeragiService sumeragiService;
  connection::ordering::OrderingService orderingService;

  connection::ServerRunner serverRunner({
      &commandService,
      &queryService,
      &sumeragiService,
      &orderingService
  },"0.0.0.0",50051);

  return 0;
}
