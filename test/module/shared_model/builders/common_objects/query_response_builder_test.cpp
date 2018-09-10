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

#include "builders/default_builders.hpp"
#include "builders/protobuf/builder_templates/query_response_template.hpp"
#include "builders/protobuf/query_responses/proto_block_query_response_builder.hpp"
#include "builders/query_responses/block_query_response_builder.hpp"
#include "cryptography/keypair.hpp"
#include "framework/specified_visitor.hpp"
#include "interfaces/common_objects/types.hpp"
#include "module/shared_model/builders/protobuf/common_objects/proto_account_asset_builder.hpp"
#include "module/shared_model/builders/protobuf/common_objects/proto_account_builder.hpp"
#include "module/shared_model/builders/protobuf/test_block_builder.hpp"
#include "module/shared_model/builders/protobuf/test_transaction_builder.hpp"
#include "utils/query_error_response_visitor.hpp"

const auto account_id = "test@domain";
const auto asset_id = "bit#domain";
const auto domain_id = "domain";
const auto hash = std::string(32, '0');

uint64_t height = 1;
uint8_t quorum = 2;

auto valid_precision = 1;

const shared_model::interface::types::DetailType account_detail =
    "account-detail";
const auto query_hash = shared_model::interface::types::HashType("hashhash");
decltype(iroha::time::now()) created_time = iroha::time::now();

TEST(QueryResponseBuilderTest, AccountAssetResponse) {
  const auto amount = shared_model::interface::Amount("10.00");
  shared_model::proto::TemplateQueryResponseBuilder<> builder;
  shared_model::proto::QueryResponse query_response =
      builder.queryHash(query_hash)
          .accountAssetResponse({shared_model::proto::AccountAssetBuilder()
                                     .accountId(account_id)
                                     .assetId(asset_id)
                                     .balance(amount)
                                     .build()})
          .build();
  ASSERT_NO_THROW({
    const auto &tmp = boost::apply_visitor(
        framework::SpecifiedVisitor<
            shared_model::interface::AccountAssetResponse>(),
        query_response.get());
    const auto &asset_response = tmp.accountAssets()[0];

    ASSERT_EQ(asset_response.assetId(), asset_id);
    ASSERT_EQ(asset_response.accountId(), account_id);
    ASSERT_EQ(asset_response.balance(), amount);
    ASSERT_EQ(query_response.queryHash(), query_hash);
  });
}

TEST(QueryResponseBuilderTest, AccountDetailResponse) {
  shared_model::proto::TemplateQueryResponseBuilder<> builder;
  shared_model::proto::QueryResponse query_response =
      builder.queryHash(query_hash)
          .accountDetailResponse(account_detail)
          .build();

  ASSERT_NO_THROW({
    const auto &account_detail_response = boost::apply_visitor(
        framework::SpecifiedVisitor<
            shared_model::interface::AccountDetailResponse>(),
        query_response.get());

    ASSERT_EQ(account_detail_response.detail(), account_detail);
    ASSERT_EQ(query_response.queryHash(), query_hash);
  });
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

  ASSERT_NO_THROW({
    const auto &account_response = boost::apply_visitor(
        framework::SpecifiedVisitor<shared_model::interface::AccountResponse>(),
        query_response.get());

    ASSERT_EQ(account_response.account(), account);
    ASSERT_EQ(account_response.roles(), roles);
    ASSERT_EQ(query_response.queryHash(), query_hash);
  });
}

template <typename T>
class ErrorResponseTest : public ::testing::Test {};

using ErrorResponseTypes =
    ::testing::Types<shared_model::interface::StatelessFailedErrorResponse,
                     shared_model::interface::StatefulFailedErrorResponse,
                     shared_model::interface::NoAccountErrorResponse,
                     shared_model::interface::NoAccountAssetsErrorResponse,
                     shared_model::interface::NoAccountDetailErrorResponse,
                     shared_model::interface::NoSignatoriesErrorResponse,
                     shared_model::interface::NotSupportedErrorResponse,
                     shared_model::interface::NoAssetErrorResponse,
                     shared_model::interface::NoRolesErrorResponse>;
TYPED_TEST_CASE(ErrorResponseTest, ErrorResponseTypes);

TYPED_TEST(ErrorResponseTest, TypeErrorResponse) {
  shared_model::proto::TemplateQueryResponseBuilder<> builder;
  shared_model::proto::QueryResponse query_response =
      builder.queryHash(query_hash).errorQueryResponse<TypeParam>().build();

  ASSERT_TRUE(boost::apply_visitor(
      shared_model::interface::QueryErrorResponseChecker<TypeParam>(),
      query_response.get()));

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

  ASSERT_NO_THROW({
    const auto &signatories_response = boost::apply_visitor(
        framework::SpecifiedVisitor<
            shared_model::interface::SignatoriesResponse>(),
        query_response.get());

    const auto &resp_keys = signatories_response.keys();
    ASSERT_EQ(keys.size(), resp_keys.size());

    for (auto i = 0u; i < keys.size(); i++) {
      ASSERT_EQ(keys.at(i).blob(), resp_keys.at(i).blob());
    }
    ASSERT_EQ(query_response.queryHash(), query_hash);
  });
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

  ASSERT_NO_THROW({
    const auto &transactions_response = boost::apply_visitor(
        framework::SpecifiedVisitor<
            shared_model::interface::TransactionsResponse>(),
        query_response.get());

    const auto &txs = transactions_response.transactions();

    ASSERT_EQ(txs.size(), 1);
    ASSERT_EQ(txs.back(), transaction);
    ASSERT_EQ(query_response.queryHash(), query_hash);
  });
}

TEST(QueryResponseBuilderTest, AssetResponse) {
  shared_model::proto::TemplateQueryResponseBuilder<> builder;
  shared_model::proto::QueryResponse query_response =
      builder.queryHash(query_hash)
          .assetResponse(asset_id, domain_id, valid_precision)
          .build();

  ASSERT_NO_THROW({
    const auto &asset_response = boost::apply_visitor(
        framework::SpecifiedVisitor<shared_model::interface::AssetResponse>(),
        query_response.get());

    const auto &asset = asset_response.asset();
    ASSERT_EQ(asset.assetId(), asset_id);
    ASSERT_EQ(asset.domainId(), domain_id);
    ASSERT_EQ(asset.precision(), valid_precision);
    ASSERT_EQ(query_response.queryHash(), query_hash);
  });
}

TEST(QueryResponseBuilderTest, RolesResponse) {
  const std::vector<std::string> roles = {"role1", "role2", "role3"};

  shared_model::proto::TemplateQueryResponseBuilder<> builder;
  shared_model::proto::QueryResponse query_response =
      builder.queryHash(query_hash).rolesResponse(roles).build();

  ASSERT_NO_THROW({
    const auto &roles_response = boost::apply_visitor(
        framework::SpecifiedVisitor<shared_model::interface::RolesResponse>(),
        query_response.get());

    ASSERT_EQ(roles_response.roles(), roles);
    ASSERT_EQ(query_response.queryHash(), query_hash);
  });
}

TEST(QueryResponseBuilderTest, RolePermissionsResponse) {
  shared_model::interface::RolePermissionSet permissions(
      {shared_model::interface::permissions::Role::kAppendRole,
       shared_model::interface::permissions::Role::kAddAssetQty,
       shared_model::interface::permissions::Role::kAddPeer});

  shared_model::proto::TemplateQueryResponseBuilder<> builder;
  shared_model::proto::QueryResponse query_response =
      builder.queryHash(query_hash)
          .rolePermissionsResponse(permissions)
          .build();

  ASSERT_NO_THROW({
    const auto &role_permissions_response = boost::apply_visitor(
        framework::SpecifiedVisitor<
            shared_model::interface::RolePermissionsResponse>(),
        query_response.get());

    ASSERT_EQ(role_permissions_response.rolePermissions(), permissions);
    ASSERT_EQ(query_response.queryHash(), query_hash);
  });
}

/**
 * @given ready block
 * @when response builder builds block_response object with that block
 * @then original block and block in block response are equal
 */
TEST(QueryResponseBuilderTest, BlockQueryResponse) {
  auto transaction = TestTransactionBuilder()
                         .createdTime(created_time)
                         .creatorAccountId(account_id)
                         .setAccountQuorum(account_id, quorum)
                         .build();
  shared_model::builder::BlockQueryResponseBuilder<
      shared_model::proto::BlockQueryResponseBuilder>
      builder;
  auto block = TestBlockBuilder()
                   .height(3)
                   .createdTime(created_time)
                   .prevHash(query_hash)
                   .transactions(std::vector<shared_model::proto::Transaction>{
                       transaction})
                   .build();
  auto query_response = builder.blockResponse(block).build();

  ASSERT_NO_THROW({
    const auto &block_response = boost::apply_visitor(
        framework::SpecifiedVisitor<shared_model::interface::BlockResponse>(),
        query_response->get());

    ASSERT_EQ(block, block_response.block());
  });
}

/**
 * @given response builder
 * @when response builder builds error response with some error message
 * @then block response with that message is built
 */
TEST(QueryResponseBuilderTest, BlockQueryErrorResponse) {
  shared_model::builder::BlockQueryResponseBuilder<
      shared_model::proto::BlockQueryResponseBuilder>
      builder;
  std::string message("some error message");
  auto query_response = builder.errorResponse(message).build();

  ASSERT_NO_THROW({
    const auto &block_response =
        boost::apply_visitor(framework::SpecifiedVisitor<
                                 shared_model::interface::BlockErrorResponse>(),
                             query_response->get());
    ASSERT_EQ(message, block_response.message());
  });
}
