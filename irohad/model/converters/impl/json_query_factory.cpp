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

#include "model/converters/json_query_factory.hpp"
#include <algorithm>

namespace iroha {
  namespace model {
    namespace converters {

      using namespace rapidjson;

      JsonQueryFactory::JsonQueryFactory()
          : log_(logger::log("JsonQueryFactory")) {
        deserializers_["GetAccount"] = &JsonQueryFactory::deserializeGetAccount;
        deserializers_["GetAccountAsset"] =
            &JsonQueryFactory::deserializeGetAccountAssets;
        deserializers_["GetAccountAssetTransactions"] =
            &JsonQueryFactory::deserializeGetAccountAssetTransactions;
        deserializers_["GetAccountTransactions"] =
            &JsonQueryFactory::deserializeGetAccountTransactions;
        deserializers_["GetAccountSignatories"] =
            &JsonQueryFactory::deserializeGetSignatories;
      }

      nonstd::optional<iroha::protocol::Query> JsonQueryFactory::deserialize(
          const std::string query_json) {
        try {
          log_->info("Deserialize query json");
          iroha::protocol::Query pb_query;
          Document doc;
          if (doc.Parse(query_json.c_str()).HasParseError()) {
            log_->error("Json is ill-formed");
            return nonstd::nullopt;
          }
          // check if all necessary fields are there
          auto obj_query = doc.GetObject();
          auto req_fields = {"signature", "creator_account_id", "created_ts",
                             "query_counter", "query_type"};
          if (std::any_of(req_fields.begin(), req_fields.end(),
                          [&obj_query](auto &&field) {
                            return not obj_query.HasMember(field);
                          })) {
            log_->error("No required fields in json");
            return nonstd::nullopt;
          }

          auto sig = obj_query["signature"].GetObject();

          // check if signature has all needed fields
          if (not sig.HasMember("pubkey")) {
            log_->error("No pubkey in signature in json");
            return nonstd::nullopt;
          }
          if (not sig.HasMember("signature")) {
            log_->error("No signature in json");
            return nonstd::nullopt;
          }

          auto pb_header = pb_query.mutable_header();
          pb_header->set_created_time(obj_query["created_ts"].GetUint64());
          auto pb_sig = pb_header->mutable_signature();

          // throws std::invalid_argument if pubkey or signature is bad
          ed25519::pubkey_t _pub =
              ed25519::pubkey_t::from_hexstring(sig["pubkey"].GetString());
          ed25519::sig_t _sig =
              ed25519::sig_t::from_hexstring(sig["signature"].GetString());

          pb_sig->set_pubkey(_pub.to_string());
          pb_sig->set_signature(_sig.to_string());

          // set creator account id
          pb_query.set_creator_account_id(
              obj_query["creator_account_id"].GetString());

          // set query counter
          pb_query.set_query_counter(obj_query["query_counter"].GetUint64());

          auto query_type = obj_query["query_type"].GetString();

          auto it = deserializers_.find(query_type);
          if (it != deserializers_.end() and
              (this->*it->second)(obj_query, pb_query)) {
            return pb_query;
          } else {
            return nonstd::nullopt;
          }
        } catch (const std::invalid_argument &e) {
          log_->error(e.what());
          return nonstd::nullopt;
        }
      }

      bool JsonQueryFactory::deserializeGetAccount(
          rapidjson::GenericValue<rapidjson::UTF8<char>>::Object &obj_query,
          protocol::Query &pb_query) {
        if (not obj_query.HasMember("account_id")) {
          log_->error("No account id in json");
          return false;
        }
        auto pb_get_account = pb_query.mutable_get_account();
        pb_get_account->set_account_id(obj_query["account_id"].GetString());

        return true;
      }

      bool JsonQueryFactory::deserializeGetSignatories(
          rapidjson::GenericValue<rapidjson::UTF8<char>>::Object &obj_query,
          protocol::Query &pb_query) {
        if (not obj_query.HasMember("account_id")) {
          log_->error("No account id in json");
          return false;
        }
        auto pb_get_signatories = pb_query.mutable_get_account_signatories();
        pb_get_signatories->set_account_id(obj_query["account_id"].GetString());

        return true;
      }

      bool JsonQueryFactory::deserializeGetAccountTransactions(
          rapidjson::GenericValue<rapidjson::UTF8<char>>::Object &obj_query,
          protocol::Query &pb_query) {
        if (not obj_query.HasMember("account_id")) {
          log_->error("No account id in json");
          return false;
        }
        auto pb_get_account_transactions =
            pb_query.mutable_get_account_transactions();
        pb_get_account_transactions->set_account_id(
            obj_query["account_id"].GetString());

        return true;
      }

      bool JsonQueryFactory::deserializeGetAccountAssetTransactions(
          rapidjson::GenericValue<rapidjson::UTF8<char>>::Object &obj_query,
          protocol::Query &pb_query) {
        if (not(obj_query.HasMember("account_id") &&
                obj_query.HasMember("asset_id"))) {
          log_->error("No account, asset id in json");
          return false;
        }

        auto pb_get_account_asset_transactions =
            pb_query.mutable_get_account_asset_transactions();
        pb_get_account_asset_transactions->set_account_id(
            obj_query["account_id"].GetString());
        pb_get_account_asset_transactions->set_asset_id(
            obj_query["asset_id"].GetString());

        return true;
      }

      bool JsonQueryFactory::deserializeGetAccountAssets(
          rapidjson::GenericValue<rapidjson::UTF8<char>>::Object &obj_query,
          protocol::Query &pb_query) {
        if (not(obj_query.HasMember("account_id") &&
                obj_query.HasMember("asset_id"))) {
          log_->error("No account, asset id in json");
          return false;
        }
        auto pb_get_account_assets = pb_query.mutable_get_account_assets();
        pb_get_account_assets->set_account_id(
            obj_query["account_id"].GetString());
        pb_get_account_assets->set_asset_id(obj_query["asset_id"].GetString());

        return true;
      }
    }  // namespace converters
  }    // namespace model
}  // namespace iroha
