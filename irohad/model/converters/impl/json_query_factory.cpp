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

#define RAPIDJSON_HAS_STDSTRING 1

#include "model/converters/json_query_factory.hpp"
#include "common/types.hpp"
#include "model/converters/json_common.hpp"

namespace iroha {
  namespace model {
    namespace converters {

      using namespace rapidjson;

      JsonQueryFactory::JsonQueryFactory()
          : log_(logger::log("JsonQueryFactory")) {
        deserializers_["GetAccount"] = &JsonQueryFactory::deserializeGetAccount;
        deserializers_["GetAccountAssets"] = &JsonQueryFactory::deserializeGetAccountAssets;
        deserializers_["GetAccountTransactions"] = &JsonQueryFactory::deserializeGetAccountTransactions;
        deserializers_["GetAccountAssetTransactions"] = &JsonQueryFactory::deserializeGetAccountAssetTransactions;
        deserializers_["GetAccountSignatories"] = &JsonQueryFactory::deserializeGetSignatories;
        // Serializers
        serializers_[typeid(GetAccount)] = &JsonQueryFactory::serializeGetAccount;
        serializers_[typeid(GetSignatories)] = &JsonQueryFactory::serializeGetSignatories;
        serializers_[typeid(GetAccountAssets)] = &JsonQueryFactory::serializeGetAccountAssets;
        serializers_[typeid(GetAccountTransactions)] = &JsonQueryFactory::serializeGetAccountTransactions;
        serializers_[typeid(GetAccountAssetTransactions)] = &JsonQueryFactory::serializeGetAccountAssetTransactions;
      }

      optional_ptr<model::Query> JsonQueryFactory::deserialize(
          const std::string query_json) {
        log_->info("Deserialize query json");
        Document doc;
        if (doc.Parse(query_json.c_str()).HasParseError()) {
          log_->error("Json is ill-formed");
          return nonstd::nullopt;
        }

        auto obj_query = doc.GetObject();
        // check if all necessary fields are there
        {
          auto req_fields = {"signature", "creator_account_id", "created_ts",
                             "query_counter", "query_type"};
          if (std::any_of(req_fields.begin(), req_fields.end(),
                          [&obj_query](auto &&field) {
                            return not obj_query.HasMember(field);
                          })) {
            log_->error("No required fields in json");
            return nonstd::nullopt;
          }
        }

        // check if all member values have valid type.
        {
          if (not obj_query["signature"].IsObject()) {
            log_->error("Type mismatch in json,  signature must be object");
            return nonstd::nullopt;
          }

          if (not obj_query["creator_account_id"].IsString()) {
            log_->error("Type mismatch in json,  created_js must be string");
            return nonstd::nullopt;
          }

          if (not obj_query["created_ts"].IsUint64()) {
            log_->error("Type mismatch in json,  created_ts must be uint64");
            return nonstd::nullopt;
          }

          if (not obj_query["query_counter"].IsUint64()) {
            log_->error("Type mismatch in json,  query_counter must be uint64");
            return nonstd::nullopt;
          }

          if (not obj_query["query_type"].IsString()) {
            log_->error("Type mismatch in json,  query_type must be string");
            return nonstd::nullopt;
          }
        }

        auto sig = obj_query["signature"].GetObject();

        // check if signature has all needed fields
        {
          if (not sig.HasMember("pubkey")) {
            log_->error("No pubkey in signature in json");
            return nonstd::nullopt;
          }
          if (not sig.HasMember("signature")) {
            log_->error("No signature in json");
            return nonstd::nullopt;
          }
        }

        // check if members of signature have valid type.
        {
          if (not sig["pubkey"].IsString()) {
            log_->error(
                "Type mismatch in json,  pubkey in signature must be string");
            return nonstd::nullopt;
          }
          if (not sig["signature"].IsString()) {
            log_->error(
                "Type mismatch in json,  signature in signature must be string");
            return nonstd::nullopt;
          }
        }

        auto query_type = obj_query["query_type"].GetString();
        auto it = deserializers_.find(query_type);
        if (it != deserializers_.end()) {
          auto query = (this->*it->second)(obj_query);
          if (not query) {
            return nonstd::nullopt;
          }

          // Set query signature
          Signature signature;
          hexstringToArray(sig["pubkey"].GetString(), signature.pubkey);
          hexstringToArray(sig["signature"].GetString(), signature.signature);
          query->signature = signature;
          return query;
        }
        log_->error("No query type found");
        return nonstd::nullopt;
      }

      std::shared_ptr<iroha::model::Query>
      JsonQueryFactory::deserializeGetAccount(
          rapidjson::GenericValue<rapidjson::UTF8<char>>::Object &obj_query) {
        if (not obj_query.HasMember("account_id")) {
          log_->error("No account id in json");
          return nullptr;
        }
        if (not obj_query["account_id"].IsString()) {
          log_->error("Type mismatch account_id in json");
          return nullptr;
        }
        auto timestamp = obj_query["created_ts"].GetUint64();
        auto creator = obj_query["creator_account_id"].GetString();
        auto counter = obj_query["query_counter"].GetUint64();
        auto account_id = obj_query["account_id"].GetString();
        return query_generator_.generateGetAccount(timestamp, creator, counter,
                                                   account_id);
      }

      std::shared_ptr<iroha::model::Query>
      JsonQueryFactory::deserializeGetSignatories(
          rapidjson::GenericValue<rapidjson::UTF8<char>>::Object &obj_query) {
        if (not obj_query.HasMember("account_id")) {
          log_->error("No account id in json");
          return nullptr;
        }
        if (not obj_query["account_id"].IsString()) {
          log_->error("Type mismatch account_id in json");
          return nullptr;
        }

        auto timestamp = obj_query["created_ts"].GetUint64();
        auto creator = obj_query["creator_account_id"].GetString();
        auto counter = obj_query["query_counter"].GetUint64();
        auto account_id = obj_query["account_id"].GetString();
        return query_generator_.generateGetSignatories(timestamp, creator,
                                                       counter, account_id);
      }

      std::shared_ptr<iroha::model::Query>
      JsonQueryFactory::deserializeGetAccountTransactions(
          rapidjson::GenericValue<rapidjson::UTF8<char>>::Object &obj_query) {
        if (not obj_query.HasMember("account_id")) {
          log_->error("No account id in json");
          return nullptr;
        }
        if (not obj_query["account_id"].IsString()) {
          log_->error("Type mismatch account id in json");
          return nullptr;
        }

        auto timestamp = obj_query["created_ts"].GetUint64();
        auto creator = obj_query["creator_account_id"].GetString();
        auto counter = obj_query["query_counter"].GetUint64();
        auto account_id = obj_query["account_id"].GetString();
        return query_generator_.generateGetAccountTransactions(
            timestamp, creator, counter, account_id);
      }

      std::shared_ptr<iroha::model::Query>
      JsonQueryFactory::deserializeGetAccountAssetTransactions(
          rapidjson::GenericValue<rapidjson::UTF8<char>>::Object &obj_query) {
        if (not obj_query.HasMember("account_id")) {
          log_->error("No account id in json");
          return nullptr;
        }
        if (not obj_query.HasMember("asset_id")) {
          log_->error("No asset id in json");
          return nullptr;
        }
        if (not obj_query["account_id"].IsString()) {
          log_->error("Type mismatch account id in json");
          return nullptr;
        }

        auto timestamp = obj_query["created_ts"].GetUint64();
        auto creator = obj_query["creator_account_id"].GetString();
        auto counter = obj_query["query_counter"].GetUint64();
        auto account_id = obj_query["account_id"].GetString();
        auto asset_id = obj_query["asset_id"].GetString();
        return query_generator_.generateGetAccountAssetTransactions(
            timestamp, creator, counter, account_id, asset_id);
      }

      std::shared_ptr<iroha::model::Query>
      JsonQueryFactory::deserializeGetAccountAssets(
          rapidjson::GenericValue<rapidjson::UTF8<char>>::Object &obj_query) {
        if (not(obj_query.HasMember("account_id") &&
                obj_query.HasMember("asset_id"))) {
          log_->error("No account id, asset id in json");
          return nullptr;
        }
        if (not(obj_query["account_id"].IsString() &&
                obj_query["asset_id"].IsString())) {
          log_->error("Type mismatch account, asset id in json");
          return nullptr;
        }

        auto timestamp = obj_query["created_ts"].GetUint64();
        auto creator = obj_query["creator_account_id"].GetString();
        auto counter = obj_query["query_counter"].GetUint64();
        auto account_id = obj_query["account_id"].GetString();
        auto asset_id = obj_query["asset_id"].GetString();
        return query_generator_.generateGetAccountAssets(
            timestamp, creator, counter, account_id, asset_id);
      }

      // --- Serialization:

      nonstd::optional<std::string> JsonQueryFactory::serialize(
          std::shared_ptr<Query> model_query) {
        Document doc;
        auto &allocator = doc.GetAllocator();
        doc.SetObject();
        doc.AddMember("creator_account_id", model_query->creator_account_id,
                      allocator);
        doc.AddMember("query_counter", model_query->query_counter, allocator);
        doc.AddMember("created_ts", model_query->created_ts, allocator);
        Value signature;
        signature.SetObject();
        signature.CopyFrom(serializeSignature(model_query->signature),
                           allocator);

        doc.AddMember("signature", signature, allocator);

        auto it = serializers_.find(typeid(*model_query));
        if (it != serializers_.end()) {
          (this->*it->second)(doc, model_query);
          return jsonToString(doc);
        }
        log_->error("Query type convertation not implemented");
        return nonstd::nullopt;
      }

      void JsonQueryFactory::serializeGetAccount(Document &json_doc,
                                                 std::shared_ptr<Query> query) {
        auto &allocator = json_doc.GetAllocator();
        json_doc.AddMember("query_type", "GetAccount", allocator);
        auto get_account = std::static_pointer_cast<GetAccount>(query);
        json_doc.AddMember("account_id", get_account->account_id, allocator);
      }
      void JsonQueryFactory::serializeGetAccountAssets(
          Document &json_doc, std::shared_ptr<Query> query) {
        auto &allocator = json_doc.GetAllocator();
        json_doc.AddMember("query_type", "GetAccountAssets", allocator);
        auto casted_query = std::static_pointer_cast<GetAccountAssets>(query);
        json_doc.AddMember("account_id", casted_query->account_id, allocator);
        json_doc.AddMember("asset_id", casted_query->asset_id, allocator);
      }

      void JsonQueryFactory::serializeGetSignatories(
          Document &json_doc, std::shared_ptr<Query> query) {
        auto &allocator = json_doc.GetAllocator();
        json_doc.AddMember("query_type", "GetAccountSignatories", allocator);
        auto get_account = std::static_pointer_cast<GetSignatories>(query);
        json_doc.AddMember("account_id", get_account->account_id, allocator);
      }

      void JsonQueryFactory::serializeGetAccountTransactions(
          Document &json_doc, std::shared_ptr<Query> query) {
        auto &allocator = json_doc.GetAllocator();
        json_doc.AddMember("query_type", "GetAccountTransactions", allocator);
        auto get_account =
            std::static_pointer_cast<GetAccountTransactions>(query);
        json_doc.AddMember("account_id", get_account->account_id, allocator);
      }

      void JsonQueryFactory::serializeGetAccountAssetTransactions(
          Document &json_doc, std::shared_ptr<Query> query) {
        auto &allocator = json_doc.GetAllocator();
        json_doc.AddMember("query_type", "GetAccountAssetTransactions", allocator);
        auto get_account_asset =
            std::static_pointer_cast<GetAccountAssetTransactions>(query);
        json_doc.AddMember("account_id", get_account_asset->account_id, allocator);
        json_doc.AddMember("asset_id", get_account_asset->asset_id, allocator);
      }

    }  // namespace converters
  }    // namespace model
}  // namespace iroha
