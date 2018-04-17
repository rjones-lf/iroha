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

#include <gtest/gtest.h>

#include "datetime/time.hpp"

#include "builders/protobuf/builder_templates/query_response_template.hpp"
#include "builders/protobuf/common_objects/proto_account_builder.hpp"
#include "builders/protobuf/common_objects/proto_amount_builder.hpp"
#include "cryptography/keypair.hpp"
#include "interfaces/common_objects/types.hpp"
#include "module/shared_model/builders/protobuf/test_transaction_builder.hpp"

const auto account_id = "test@domain";
const auto asset_id = "bit#domain";
const auto domain_id = "domain";
const auto hash = std::string(32, '0');

uint64_t height = 1;
uint8_t quorum = 2;

boost::multiprecision::uint256_t valid_value = 1000;
auto valid_precision = 1;
const iroha::Amount amount(valid_value, valid_precision);
const auto proto_amount = shared_model::proto::AmountBuilder()
                              .intValue(valid_value)
                              .precision(valid_precision)
                              .build();
const shared_model::interface::types::DetailType account_detail =
    "account-detail";
const auto query_hash = shared_model::interface::types::HashType("hashhash");
decltype(iroha::time::now()) created_time = iroha::time::now();

template <class T>
using w = shared_model::detail::PolymorphicWrapper<T>;

TEST(QueryResponseBuilderTest, AccountAssetResponse) {
  shared_model::proto::TemplateQueryResponseBuilder<> builder;
  shared_model::proto::QueryResponse query_response =
      builder.queryHash(query_hash)
          .accountAssetResponse(asset_id, account_id, proto_amount)
          .build();

  const auto tmp = boost::get<w<shared_model::interface::AccountAssetResponse>>(
      query_response.get());
  const auto &asset_response = tmp->accountAsset();

  ASSERT_EQ(asset_response.assetId(), asset_id);
  ASSERT_EQ(asset_response.accountId(), account_id);
  ASSERT_EQ(asset_response.balance(), proto_amount);
  ASSERT_EQ(query_response.queryHash(), query_hash);
}

TEST(QueryResponseBuilderTest, AccountDetailResponse) {
  shared_model::proto::TemplateQueryResponseBuilder<> builder;
  shared_model::proto::QueryResponse query_response =
      builder.queryHash(query_hash)
          .accountDetailResponse(account_detail)
          .build();

  const auto account_detail_response =
      boost::get<w<shared_model::interface::AccountDetailResponse>>(
          query_response.get());

  ASSERT_EQ(account_detail_response->detail(), account_detail);
  ASSERT_EQ(query_response.queryHash(), query_hash);
}

TEST(QueryResponseBuilderTest, AccountResponse) {
  auto valid_account_id = "name@domain";
  auto valid_domain_id = "america";
  auto valid_quorum = 3;
  auto valid_json_data = "{}";
  const std::vector<std::string> roles = {"role1", "role2"};

  auto account = shared_model::proto::AccountBuilder()
                     .accountId(valid_account_id)
                     .domainId(valid_domain_id)
                     .quorum(valid_quorum)
                     .jsonData(valid_json_data)
                     .build();

  shared_model::proto::TemplateQueryResponseBuilder<> builder;
  shared_model::proto::QueryResponse query_response =
      builder.queryHash(query_hash).accountResponse(account, roles).build();

  const auto account_response =
      boost::get<w<shared_model::interface::AccountResponse>>(
          query_response.get());

  ASSERT_EQ(account_response->account(), account);
  ASSERT_EQ(account_response->roles(), roles);
  ASSERT_EQ(query_response.queryHash(), query_hash);
}

TEST(QueryResponseBuilderTest, StatefulFailedErrorResponse) {
  shared_model::proto::TemplateQueryResponseBuilder<> builder;
  shared_model::proto::QueryResponse query_response =
      builder.queryHash(query_hash)
          .errorQueryResponse<
              shared_model::interface::StatefulFailedErrorResponse>()
          .build();

  const auto error_response =
      boost::get<w<shared_model::interface::ErrorQueryResponse>>(
          query_response.get());

  ASSERT_NO_THROW(
      boost::get<w<shared_model::interface::StatefulFailedErrorResponse>>(
          error_response->get()));

  ASSERT_ANY_THROW(
      boost::get<w<shared_model::interface::StatelessFailedErrorResponse>>(
          error_response->get()));

  ASSERT_EQ(query_response.queryHash(), query_hash);
}

TEST(QueryResponseBuilderTest, StatelessFailedErrorResponse) {
  shared_model::proto::TemplateQueryResponseBuilder<> builder;
  shared_model::proto::QueryResponse query_response =
      builder.queryHash(query_hash)
          .errorQueryResponse<
              shared_model::interface::StatelessFailedErrorResponse>()
          .build();

  const auto error_response =
      boost::get<w<shared_model::interface::ErrorQueryResponse>>(
          query_response.get());

  ASSERT_NO_THROW(
      boost::get<w<shared_model::interface::StatelessFailedErrorResponse>>(
          error_response->get()));

  ASSERT_ANY_THROW(
      boost::get<w<shared_model::interface::StatefulFailedErrorResponse>>(
          error_response->get()));

  ASSERT_EQ(query_response.queryHash(), query_hash);
}

TEST(QueryResponseBuilderTest, NoAccountErrorResponse) {
  shared_model::proto::TemplateQueryResponseBuilder<> builder;
  shared_model::proto::QueryResponse query_response =
      builder.queryHash(query_hash)
          .errorQueryResponse<shared_model::interface::NoAccountErrorResponse>()
          .build();

  const auto error_response =
      boost::get<w<shared_model::interface::ErrorQueryResponse>>(
          query_response.get());

  ASSERT_NO_THROW(
      boost::get<w<shared_model::interface::NoAccountErrorResponse>>(
          error_response->get()));

  ASSERT_ANY_THROW(
      boost::get<w<shared_model::interface::StatelessFailedErrorResponse>>(
          error_response->get()));

  ASSERT_EQ(query_response.queryHash(), query_hash);
}

TEST(QueryResponseBuilderTest, NoAccountAssetsErrorResponse) {
  shared_model::proto::TemplateQueryResponseBuilder<> builder;
  shared_model::proto::QueryResponse query_response =
      builder.queryHash(query_hash)
          .errorQueryResponse<
              shared_model::interface::NoAccountAssetsErrorResponse>()
          .build();

  const auto error_response =
      boost::get<w<shared_model::interface::ErrorQueryResponse>>(
          query_response.get());

  ASSERT_NO_THROW(
      boost::get<w<shared_model::interface::NoAccountAssetsErrorResponse>>(
          error_response->get()));

  ASSERT_ANY_THROW(
      boost::get<w<shared_model::interface::StatelessFailedErrorResponse>>(
          error_response->get()));

  ASSERT_EQ(query_response.queryHash(), query_hash);
}

TEST(QueryResponseBuilderTest, NoAccountDetailErrorResponse) {
  shared_model::proto::TemplateQueryResponseBuilder<> builder;
  shared_model::proto::QueryResponse query_response =
      builder.queryHash(query_hash)
          .errorQueryResponse<
              shared_model::interface::NoAccountDetailErrorResponse>()
          .build();

  const auto error_response =
      boost::get<w<shared_model::interface::ErrorQueryResponse>>(
          query_response.get());

  ASSERT_NO_THROW(
      boost::get<w<shared_model::interface::NoAccountDetailErrorResponse>>(
          error_response->get()));

  ASSERT_ANY_THROW(
      boost::get<w<shared_model::interface::StatelessFailedErrorResponse>>(
          error_response->get()));

  ASSERT_EQ(query_response.queryHash(), query_hash);
}

TEST(QueryResponseBuilderTest, NoSignatoriesErrorResponse) {
  shared_model::proto::TemplateQueryResponseBuilder<> builder;
  shared_model::proto::QueryResponse query_response =
      builder.queryHash(query_hash)
          .errorQueryResponse<
              shared_model::interface::NoSignatoriesErrorResponse>()
          .build();

  const auto error_response =
      boost::get<w<shared_model::interface::ErrorQueryResponse>>(
          query_response.get());

  ASSERT_NO_THROW(
      boost::get<w<shared_model::interface::NoSignatoriesErrorResponse>>(
          error_response->get()));

  ASSERT_ANY_THROW(
      boost::get<w<shared_model::interface::StatelessFailedErrorResponse>>(
          error_response->get()));

  ASSERT_EQ(query_response.queryHash(), query_hash);
}

TEST(QueryResponseBuilderTest, NotSupportedErrorResponse) {
  shared_model::proto::TemplateQueryResponseBuilder<> builder;
  shared_model::proto::QueryResponse query_response =
      builder.queryHash(query_hash)
          .errorQueryResponse<
              shared_model::interface::NotSupportedErrorResponse>()
          .build();

  const auto error_response =
      boost::get<w<shared_model::interface::ErrorQueryResponse>>(
          query_response.get());

  ASSERT_NO_THROW(
      boost::get<w<shared_model::interface::NotSupportedErrorResponse>>(
          error_response->get()));

  ASSERT_ANY_THROW(
      boost::get<w<shared_model::interface::StatelessFailedErrorResponse>>(
          error_response->get()));

  ASSERT_EQ(query_response.queryHash(), query_hash);
}

TEST(QueryResponseBuilderTest, NoAssetErrorResponse) {
  shared_model::proto::TemplateQueryResponseBuilder<> builder;
  shared_model::proto::QueryResponse query_response =
      builder.queryHash(query_hash)
          .errorQueryResponse<shared_model::interface::NoAssetErrorResponse>()
          .build();

  const auto error_response =
      boost::get<w<shared_model::interface::ErrorQueryResponse>>(
          query_response.get());

  ASSERT_NO_THROW(boost::get<w<shared_model::interface::NoAssetErrorResponse>>(
      error_response->get()));

  ASSERT_ANY_THROW(
      boost::get<w<shared_model::interface::StatelessFailedErrorResponse>>(
          error_response->get()));

  ASSERT_EQ(query_response.queryHash(), query_hash);
}

TEST(QueryResponseBuilderTest, NoRolesErrorResponse) {
  shared_model::proto::TemplateQueryResponseBuilder<> builder;
  shared_model::proto::QueryResponse query_response =
      builder.queryHash(query_hash)
          .errorQueryResponse<shared_model::interface::NoRolesErrorResponse>()
          .build();

  const auto error_response =
      boost::get<w<shared_model::interface::ErrorQueryResponse>>(
          query_response.get());

  ASSERT_NO_THROW(boost::get<w<shared_model::interface::NoRolesErrorResponse>>(
      error_response->get()));

  ASSERT_ANY_THROW(
      boost::get<w<shared_model::interface::StatelessFailedErrorResponse>>(
          error_response->get()));

  ASSERT_EQ(query_response.queryHash(), query_hash);
}

TEST(QueryResponseBuilderTest, SignatoriesResponse) {
  std::vector<shared_model::interface::types::PubkeyType> keys = {
      shared_model::interface::types::PubkeyType("key1"),
      shared_model::interface::types::PubkeyType("key1"),
      shared_model::interface::types::PubkeyType("key1")};

  shared_model::proto::TemplateQueryResponseBuilder<> builder;
  shared_model::proto::QueryResponse query_response =
      builder.queryHash(query_hash).signatoriesResponse(keys).build();

  const auto signatories_response =
      boost::get<w<shared_model::interface::SignatoriesResponse>>(
          query_response.get());

  const auto &resp_keys = signatories_response->keys();
  ASSERT_EQ(keys.size(), resp_keys.size());

  for (auto i = 0u; i < keys.size(); i++) {
    ASSERT_EQ(keys.at(i).blob(), resp_keys.at(i)->blob());
  }
  ASSERT_EQ(query_response.queryHash(), query_hash);
}

TEST(QueryResponseBuilderTest, TransactionsResponse) {
  auto transaction = TestTransactionBuilder()
                         .createdTime(created_time)
                         .creatorAccountId(account_id)
                         .setAccountQuorum(account_id, quorum)
                         .build();

  shared_model::proto::TemplateQueryResponseBuilder<> builder;
  shared_model::proto::QueryResponse query_response =
      builder.queryHash(query_hash).transactionsResponse({transaction}).build();

  const auto transactions_response =
      boost::get<w<shared_model::interface::TransactionsResponse>>(
          query_response.get());

  const auto &txs = transactions_response->transactions();

  ASSERT_EQ(txs.size(), 1);
  ASSERT_EQ(*txs.back(), transaction);
  ASSERT_EQ(query_response.queryHash(), query_hash);
}

TEST(QueryResponseBuilderTest, AssetResponse) {
  shared_model::proto::TemplateQueryResponseBuilder<> builder;
  shared_model::proto::QueryResponse query_response =
      builder.queryHash(query_hash)
          .assetResponse(asset_id, domain_id, valid_precision)
          .build();

  const auto asset_response =
      boost::get<w<shared_model::interface::AssetResponse>>(
          query_response.get());

  const auto &asset = asset_response->asset();
  ASSERT_EQ(asset.assetId(), asset_id);
  ASSERT_EQ(asset.domainId(), domain_id);
  ASSERT_EQ(asset.precision(), valid_precision);
  ASSERT_EQ(query_response.queryHash(), query_hash);
}

TEST(QueryResponseBuilderTest, RolesResponse) {
  const std::vector<std::string> roles = {"role1", "role2", "role3"};

  shared_model::proto::TemplateQueryResponseBuilder<> builder;
  shared_model::proto::QueryResponse query_response =
      builder.queryHash(query_hash).rolesResponse(roles).build();

  const auto roles_response =
      boost::get<w<shared_model::interface::RolesResponse>>(
          query_response.get());

  ASSERT_EQ(roles_response->roles(), roles);
  ASSERT_EQ(query_response.queryHash(), query_hash);
}

TEST(QueryResponseBuilderTest, RolePermissionsResponse) {
  const std::vector<std::string> roles = {"role1", "role2", "role3"};

  shared_model::proto::TemplateQueryResponseBuilder<> builder;
  shared_model::proto::QueryResponse query_response =
      builder.queryHash(query_hash).rolePermissionsResponse(roles).build();

  const auto role_permissions_response =
      boost::get<w<shared_model::interface::RolePermissionsResponse>>(
          query_response.get());

  ASSERT_EQ(role_permissions_response->rolePermissions(), roles);
  ASSERT_EQ(query_response.queryHash(), query_hash);
}
