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

#ifndef IROHA_JSON_QUERY_FACTORY_HPP
#define IROHA_JSON_QUERY_FACTORY_HPP

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include "model/common.hpp"
#include <memory>
#include <nonstd/optional.hpp>
#include <typeindex>
#include <unordered_map>
#include "model/query.hpp"
#include "queries.pb.h"

#include "logger/logger.hpp"
#include "model/generators/query_generator.hpp"

namespace iroha {
  namespace model {
    namespace converters {
      using namespace rapidjson;

      class JsonQueryFactory {
       public:
        JsonQueryFactory();

        /**
         * get query from string json
         * @param query_json string representation of query
         * @return deserialized query
         */
        optional_ptr <Query> deserialize(const std::string query_json);

        /**
         * Convert model Query to json string
         * @param model_query - model representation of query
         * @return serialized Query in json format
         */
        nonstd::optional<std::string> serialize(
            std::shared_ptr<Query> model_query);

       private:
        using Deserializer = std::shared_ptr<Query> (JsonQueryFactory::*)(
            GenericValue<UTF8<char>>::Object &);

        std::unordered_map<std::string, Deserializer> deserializers_;
        // Deserialize handlers
        std::shared_ptr<Query> deserializeGetAccount(GenericValue<UTF8<char>>::Object &obj_query);
        std::shared_ptr<Query> deserializeGetSignatories(GenericValue<UTF8<char>>::Object &obj_query);
        std::shared_ptr<Query> deserializeGetAccountTransactions(GenericValue<UTF8<char>>::Object &obj_query);
        std::shared_ptr<Query> deserializeGetAccountAssetTransactions(GenericValue<UTF8<char>>::Object &obj_query);
        std::shared_ptr<Query> deserializeGetAccountAssets(GenericValue<UTF8<char>>::Object &obj_query);
        // Serializers:
        using Serializer = void (JsonQueryFactory::*)(Document &, std::shared_ptr<Query>);
        std::unordered_map<std::type_index, Serializer> serializers_;
        // Serialization handlers
        void serializeGetAccount(Document &json_doc, std::shared_ptr<Query> query);
        void serializeGetAccountAssets(Document &json_doc, std::shared_ptr<Query> query);
        void serializeGetAccountTransactions(Document &json_doc, std::shared_ptr<Query> query);
        void serializeGetAccountAssetTransactions(Document &json_doc, std::shared_ptr<Query> query);
        void serializeGetSignatories(Document &json_doc, std::shared_ptr<Query> query);

        // Logger
        std::shared_ptr<spdlog::logger> log_;
        generators::QueryGenerator query_generator_;
      };
    }  // namespace converters
  }    // namespace model
}  // namespace iroha

#endif  // IROHA_JSON_QUERY_FACTORY_HPP
