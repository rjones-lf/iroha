/**
 * Copyright Soramitsu Co., Ltd. 2017 All Rights Reserved.
 * http://soramitsu.co.jp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
//
// Created by bogdan on 20.06.17.
//

#ifndef IROHA_CONTEXT_H
#define IROHA_CONTEXT_H

#include <string>

namespace iroha{

// forward declarations
struct PostgresConnection;
struct Connection;
struct Context;


struct Context{
  PostgresConnection wsv;
  Connection index;
  Connection ordering;
};


struct Connection {
  std::string host;
  uint16_t port;
};


struct PostgresConnection: public Connection {
  std::string user;
  std::string password;
};

}

#endif //IROHA_CONTEXT_H
