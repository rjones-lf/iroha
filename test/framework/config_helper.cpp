/**
 * Copyright Soramitsu Co., Ltd. 2018 All Rights Reserved.
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

#include "framework/config_helper.hpp"

#include <sstream>
#include "logger/logger.hpp"

namespace integration_framework {

  std::string getPostgresCredsOrDefault(const std::string &default_conn) {
    auto pg_host = std::getenv("IROHA_POSTGRES_HOST");
    auto pg_port = std::getenv("IROHA_POSTGRES_PORT");
    auto pg_user = std::getenv("IROHA_POSTGRES_USER");
    auto pg_pass = std::getenv("IROHA_POSTGRES_PASSWORD");

    if (pg_host and pg_port and pg_user and pg_pass) {
      std::stringstream ss;
      ss << "host=" << pg_host;
      ss << " port=" << pg_port;
      ss << " user=" << pg_user;
      ss << " password=" << pg_pass;
      return ss.str();
    }
    return default_conn;
  }
}  // namespace integration_framework
