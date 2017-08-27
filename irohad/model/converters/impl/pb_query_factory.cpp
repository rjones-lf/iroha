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

#include "model/converters/pb_query_factory.hpp"
#include "model/common.hpp"
#include "model/model_hash_provider_impl.hpp"
#include "model/queries/get_account.hpp"
#include "model/queries/get_account_assets.hpp"
#include "model/queries/get_signatories.hpp"
#include "model/queries/get_transactions.hpp"

namespace iroha {
  namespace model {
    namespace converters {

      PbQueryFactory::PbQueryFactory() {
        log_ = logger::log("PbQueryFactory");
        serializers_[typeid(GetAccount)] = &PbQueryFactory::serializeGetAccount;
        serializers_[typeid(GetAccountAssets)] = &PbQueryFactory::serializeGetAccountAssets;
        serializers_[typeid(GetAccountTransactions)] = &PbQueryFactory::serializeGetAccountTransactions;
        serializers_[typeid(GetAccountAssetTransactions)] = &PbQueryFactory::serializeGetAccountAssetTransactions;
        serializers_[typeid(GetSignatories)] = &PbQueryFactory::serializeGetSignatories;
      }

      optional_ptr<model::Query> PbQueryFactory::deserialize(
          const protocol::Query& pb_query) {
        std::shared_ptr<model::Query> val;

        if (pb_query.has_get_account()) {
          // Convert to get Account
          auto pb_cast = pb_query.get_account();
          auto account_query = GetAccount();
          account_query.account_id = pb_cast.account_id();
          val = std::make_shared<model::GetAccount>(account_query);
        }

        if (pb_query.has_get_account_assets()) {
          // Convert to get Account Asset
          auto pb_cast = pb_query.get_account_assets();
          auto query = GetAccountAssets();
          query.account_id = pb_cast.account_id();
          query.asset_id = pb_cast.asset_id();
          val = std::make_shared<model::GetAccountAssets>(query);
        }
        if (pb_query.has_get_account_signatories()) {
          // Convert to get Signatories
          auto pb_cast = pb_query.get_account_signatories();
          auto query = GetSignatories();
          query.account_id = pb_cast.account_id();
          val = std::make_shared<model::GetSignatories>(query);
        }

        if (pb_query.has_get_account_transactions()) {
          // Convert to get Signatories
          auto pb_cast = pb_query.get_account_transactions();
          auto query = GetAccountTransactions();
          query.account_id = pb_cast.account_id();
          val = std::make_shared<model::GetAccountTransactions>(query);
        }

        if (pb_query.has_get_account_asset_transactions()) {
          // Convert to get Signatories
          auto pb_cast = pb_query.get_account_asset_transactions();
          auto query = GetAccountAssetTransactions();
          query.account_id = pb_cast.account_id();
          query.asset_id = pb_cast.asset_id();
          val = std::make_shared<model::GetAccountAssetTransactions>(query);
        }

        if (!val) {
          // Query not implemented
          return nonstd::nullopt;
        }
        Signature sign;
        auto pb_sign = pb_query.header().signature();
        std::copy(pb_sign.pubkey().begin(), pb_sign.pubkey().end(),
                  sign.pubkey.begin());

        std::copy(pb_sign.signature().begin(), pb_sign.signature().end(),
                  sign.signature.begin());
        val->query_counter = pb_query.query_counter();
        val->signature = sign;
        val->created_ts = pb_query.header().created_time();
        val->creator_account_id = pb_query.creator_account_id();
        model::HashProviderImpl hashProvider;  // TODO: get rid off unnecessary
                                               // object initialization
        val->query_hash = hashProvider.get_hash(val);
        return val;
      }

      void PbQueryFactory::serializeQueryMetaData(
          protocol::Query& pb_query, std::shared_ptr<Query> query) {
        pb_query.set_creator_account_id(query->creator_account_id);
        auto header = pb_query.mutable_header();
        header->set_created_time(query->created_ts);
        // Set signatures
        auto sig = header->mutable_signature();
        sig->set_signature(query->signature.signature.to_string());
        sig->set_pubkey(query->signature.pubkey.to_string());
      }

      nonstd::optional<protocol::Query> PbQueryFactory::serialize(
          std::shared_ptr<Query> query) {
        auto it = serializers_.find(typeid(*query));
        if (it != serializers_.end()) {
          return (this->*it->second)(query);
        }
        log_->error("Query type not found");
        return nonstd::nullopt;
      }

      protocol::Query PbQueryFactory::serializeGetAccount(
          std::shared_ptr<Query> query) {
        protocol::Query pb_query;
        serializeQueryMetaData(pb_query, query);
        auto tmp = std::static_pointer_cast<GetAccount>(query);
        auto account_id = tmp->account_id;
        auto pb_query_mut = pb_query.mutable_get_account();
        pb_query_mut->set_account_id(account_id);
        return pb_query;
      };

      protocol::Query PbQueryFactory::serializeGetAccountAssets(
          std::shared_ptr<Query> query) {
        protocol::Query pb_query;
        serializeQueryMetaData(pb_query, query);
        auto tmp = std::static_pointer_cast<GetAccountAssets>(query);
        auto account_id = tmp->account_id;
        auto asset_id = tmp->asset_id;
        auto pb_query_mut = pb_query.mutable_get_account_assets();
        pb_query_mut->set_account_id(account_id);
        pb_query_mut->set_asset_id(asset_id);
        return pb_query;
      };

      protocol::Query PbQueryFactory::serializeGetAccountTransactions(
          std::shared_ptr<Query> query) {
        protocol::Query pb_query;
        serializeQueryMetaData(pb_query, query);
        auto tmp = std::static_pointer_cast<GetAccountTransactions>(query);
        auto account_id = tmp->account_id;
        auto pb_query_mut = pb_query.mutable_get_account_transactions();
        pb_query_mut->set_account_id(account_id);
        return pb_query;
      }

      protocol::Query PbQueryFactory::serializeGetAccountAssetTransactions(
          std::shared_ptr<Query> query) {
        protocol::Query pb_query;
        serializeQueryMetaData(pb_query, query);
        auto tmp = std::static_pointer_cast<GetAccountAssetTransactions>(query);
        auto account_id = tmp->account_id;
        auto asset_id = tmp->asset_id;
        auto pb_query_mut = pb_query.mutable_get_account_asset_transactions();
        pb_query_mut->set_account_id(account_id);
        pb_query_mut->set_asset_id(asset_id);
        return pb_query;
      }

      protocol::Query PbQueryFactory::serializeGetSignatories(
          std::shared_ptr<Query> query) {
        protocol::Query pb_query;
        serializeQueryMetaData(pb_query, query);
        auto tmp = std::static_pointer_cast<GetSignatories>(query);
        auto account_id = tmp->account_id;
        auto pb_query_mut = pb_query.mutable_get_account_signatories();
        pb_query_mut->set_account_id(account_id);
        return pb_query;
      }

    }  // namespace converters
  }    // namespace model
}  // namespace iroha
