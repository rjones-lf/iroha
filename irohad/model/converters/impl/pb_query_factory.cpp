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
#include <model/model_hash_provider_impl.hpp>
#include "model/queries/get_account.hpp"
#include "model/queries/get_account_assets.hpp"
#include "model/queries/get_signatories.hpp"
#include "model/queries/get_transactions.hpp"

namespace iroha {
  namespace model {
    namespace converters {

      std::shared_ptr<model::Query> PbQueryFactory::deserialize(
          const protocol::Query &pb_query) {
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
        if (!val) {
          // Query not implemented
          return nullptr;
        }
        Signature sign;
        auto pb_sign = pb_query.header().signature();
        sign.pubkey = pb_sign.pubkey();
        sign.signature = pb_sign.signature();
        val->query_counter = pb_query.query_counter();
        val->signature = sign;
        val->created_ts = pb_query.header().created_time();
        val->creator_account_id = pb_query.creator_account_id();
        model::HashProviderImpl hashProvider;  // TODO: get rid off unnecessary
                                               // object initialization
        val->query_hash = hashProvider.get_hash(val);
        return val;
      }
    }
  }
}
