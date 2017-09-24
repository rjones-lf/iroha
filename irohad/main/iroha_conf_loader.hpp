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

#ifndef IROHA_CONF_LOADER_HPP
#define IROHA_CONF_LOADER_HPP

#include <fstream>
#include <string>

#include <rapidjson/rapidjson.h>

#include "common/assert_config.hpp"
#include "common/types.hpp"
#include "model/converters/json_common.hpp"

namespace config_members {
  auto BlockStorePath = "IROHA_BLOCK_STORE_PATH";
  auto ToriiPort = "IROHA_TORII_PORT";  // TODO: Needs AddPeer.
  auto KeyPairPath = "IROHA_KEY_PAIR_PATH";
  auto PgHost = "IROHA_PG_HOST";
  auto PgPort = "IROHA_PG_PORT";
  auto PgUser = "IROHA_PG_USER";
  auto PgPass = "IROHA_PG_PASS";
  auto RedisHost = "IROHA_REDIS_HOST";
  auto RedisPort = "IROHA_REDIS_PORT";
  auto PeerNumber = "IROHA_PEER_NUMBER";
}  // namespace config_members

/**
 * Get the value of given environment variable
 * @param var - name of environment variable
 * @return value, if variable is set, nullopt otherwise
 */
auto getEnvironmentVariable(const char *var) -> nonstd::optional<const char *> {
  auto val = std::getenv(var);
  if (val) {
    return val;
  }
  return nonstd::nullopt;
}

// TODO common json library in libs directory
namespace iroha {
  namespace model {
    namespace converters {
      template <>
      struct Convert<size_t> {
        template <typename T>
        auto operator()(T &&x) -> nonstd::optional<size_t> {
          try {
            return std::stoull(x);
          } catch (const std::exception &e) {
            return nonstd::nullopt;
          }
        }
      };
    }  // namespace converters
  }    // namespace model
}  // namespace iroha

/**
 * Serialize given environment variable to document
 * @tparam T - environment variable type
 * @tparam Convert - transform function type
 * @param var - environment variable for serialization
 * @param transform - transform function from string to T
 * @return
 */
template <typename T = rapidjson::Value::StringRefType,
          typename Convert = iroha::model::converters::Convert<T>>
auto serializeValue(const char *var, Convert transform = Convert()) {
  return [=](auto &document) {
    return getEnvironmentVariable(var) | transform | [&](auto val) {
      auto &allocator = document.GetAllocator();
      document.AddMember(rapidjson::Value::StringRefType(var), val, allocator);
      std::cout << iroha::model::converters::jsonToString(document) << std::endl;
      return nonstd::make_optional(std::move(document));
    };
  };
}

/**
 * parse and assert trusted peers
 * @return rapidjson::Document
 */
inline auto parse_iroha_config() {
  using iroha::operator|;
  using namespace assert_config;
  namespace mbr = config_members;

  // TODO restore key pair path parameter when crypto is ready
  return nonstd::make_optional<rapidjson::Document>(
             rapidjson::Type::kObjectType)
      | serializeValue(mbr::BlockStorePath)
      | serializeValue<size_t>(mbr::ToriiPort) | serializeValue(mbr::PgHost)
      | serializeValue<size_t>(mbr::PgPort) | serializeValue(mbr::PgUser)
      | serializeValue(mbr::PgPass) | serializeValue(mbr::RedisHost)
      | serializeValue<size_t>(mbr::RedisPort)
      | serializeValue<size_t>(mbr::PeerNumber);
}

#endif  // IROHA_CONF_LOADER_HPP
