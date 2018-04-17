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
    if (not pg_host) {
      return default_conn;
    } else {
      std::stringstream ss;
      ss << "host=" << pg_host << " port=" << pg_port << " user=" << pg_user
         << " password=" << pg_pass;
      logger::log("ITF")->info("Postgres credentials: {}", ss.str());
      return ss.str();
    }
  }
}  // namespace integration_framework
