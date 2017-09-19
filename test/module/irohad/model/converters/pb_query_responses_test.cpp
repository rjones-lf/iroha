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

#include <gtest/gtest.h>
#include "model/converters/pb_query_response_factory.hpp"

using namespace iroha;

TEST(QueryResponseTest, AccountTest) {
  model::converters::PbQueryResponseFactory pb_factory;
  model::Account account;
  account.account_id = "123";
  account.permissions.add_signatory = true;
  account.domain_name = "domain";
  account.quorum = 32;

  auto pb_account = pb_factory.serializeAccount(account);
  auto des_account = pb_factory.deserializeAccount(pb_account);

  ASSERT_EQ(account.account_id, des_account.account_id);
  ASSERT_EQ(account.permissions, des_account.permissions);
  ASSERT_EQ(account.domain_name, des_account.domain_name);
  ASSERT_EQ(account.quorum, account.quorum);
}

TEST(QueryResponseTest, AccountResponseTest) {
  model::converters::PbQueryResponseFactory pb_factory;

  model::Account account;
  account.account_id = "123";
  account.permissions.add_signatory = true;
  account.domain_name = "domain";
  account.quorum = 32;

  model::AccountResponse accountResponse;
  accountResponse.account = account;

  auto pb_account_response =
      pb_factory.serializeAccountResponse(accountResponse);
  auto des_account_response =
      pb_factory.deserializeAccountResponse(pb_account_response);

  ASSERT_EQ(accountResponse.account.account_id,
            des_account_response.account.account_id);
  ASSERT_EQ(accountResponse.account.permissions,
            des_account_response.account.permissions);
  ASSERT_EQ(accountResponse.account.domain_name,
            des_account_response.account.domain_name);
  ASSERT_EQ(accountResponse.account.quorum, account.quorum);
}

TEST(QueryResponseTest, AccountAsset) {
  model::converters::PbQueryResponseFactory pb_factory;

  model::AccountAsset account_asset;
  account_asset.account_id = "123";
  account_asset.asset_id = "123";
  account_asset.balance = 1;

  auto pb_account_asset = pb_factory.serializeAccountAsset(account_asset);
  auto des_account_asset = pb_factory.deserializeAccountAsset(pb_account_asset);

  ASSERT_EQ(account_asset.balance, des_account_asset.balance);
  ASSERT_EQ(account_asset.asset_id, des_account_asset.asset_id);
  ASSERT_EQ(account_asset.account_id, des_account_asset.account_id);

  model::AccountAssetResponse account_asset_response;
  account_asset_response.acct_asset = account_asset;

  auto shrd_aar = std::make_shared<decltype(account_asset_response)>(
      account_asset_response);
  auto query_response = *pb_factory.serialize(shrd_aar);

  auto des_account_asset_response = pb_factory.deserializeAccountAssetResponse(
      query_response.account_assets_response());

  ASSERT_EQ(des_account_asset_response.acct_asset.balance,
            des_account_asset.balance);
  ASSERT_EQ(des_account_asset_response.acct_asset.asset_id,
            des_account_asset.asset_id);
  ASSERT_EQ(des_account_asset_response.acct_asset.account_id,
            des_account_asset.account_id);
}

TEST(QueryResponseTest, SignatoriesTest) {
  model::converters::PbQueryResponseFactory pb_factory;

  model::SignatoriesResponse signatories_response{};
  ed25519::pubkey_t pubkey;
  std::fill(pubkey.begin(), pubkey.end(), 0x1);
  signatories_response.keys.push_back(pubkey);

  auto shrd_sr =
      std::make_shared<decltype(signatories_response)>(signatories_response);
  auto query_response = *pb_factory.serialize(shrd_sr);
  auto des_signatories_response = pb_factory.deserializeSignatoriesResponse(
      query_response.signatories_response());

  ASSERT_EQ(signatories_response.keys, des_signatories_response.keys);
}

TEST(QueryResponseTest, TransactionsResponseTest) {
  model::converters::PbQueryResponseFactory pb_factory;

  model::TransactionsResponse txs_response{};

  txs_response.transactions =
          rxcpp::observable<>::iterate([] {
              std::vector<model::Transaction> result;
              for (size_t i = 0; i < 3; ++i) {
                model::Transaction current;
                current.tx_counter = i;
                result.push_back(current);
              }
              return result;
          }());

  auto shrd_tr = std::make_shared<decltype(txs_response)>(txs_response);
  auto query_response = *pb_factory.serialize(shrd_tr);

  ASSERT_EQ(query_response.transactions_response().transactions().size(), 3);
  for (size_t i = 0; i < 3; i++) {
    ASSERT_EQ(query_response.transactions_response()
                  .transactions()
                  .Get(i)
                  .meta()
                  .tx_counter(),
              i);
  }
}
