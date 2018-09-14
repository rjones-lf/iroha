/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>

#include "backend/protobuf/common_objects/proto_common_objects_factory.hpp"
#include "backend/protobuf/proto_query_response_factory.hpp"
#include "validators/field_validator.hpp"

using namespace shared_model::proto;
using shared_model::validation::FieldValidator;

class ProtoQueryResponseFactoryTest : public ::testing::Test {
 public:
  std::shared_ptr<ProtoQueryResponseFactory> response_factory;
  std::shared_ptr<ProtoCommonObjectsFactory<FieldValidator>> objects_factory;

  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(ProtoQueryResponseFactoryTest, CreateAccountAssetResponse) {
  constexpr int kAccountAssetsNumber = 5;
  const std::string kAccountId = "doge@meme";
  const std::string kAssetId = "dogecoin#iroha";

  std::vector<shared_model::interface::AccountAsset> assets;
  for (auto i = 1; i < kAccountAssetsNumber; ++i) {
    assets.emplace_back(objects_factory->createAccountAsset(
        kAccountId,
        kAssetId,
        shared_model::interface::Amount(std::to_string(i))));
  }

  auto response = response_factory->createAccountAssetResponse(assets);
  ASSERT_TRUE(response);
  ASSERT_EQ(response->accountAssets().front().accountId(), kAccountId);
  ASSERT_EQ(response->accountAssets().front().assetId(), kAssetId);
  for (auto i = 1; i < kAccountAssetsNumber; i++) {
    ASSERT_EQ(response->accountAssets()[i - 1].balance(),
              assets[i - 1].balance());
  }
}

TEST_F(ProtoQueryResponseFactoryTest, CreateAccountDetailResponse) {}

TEST_F(ProtoQueryResponseFactoryTest, CreateAccountResponse) {}

TEST_F(ProtoQueryResponseFactoryTest, CreateErrorQueryResponse) {}

TEST_F(ProtoQueryResponseFactoryTest, CreateSignatoriesResponse) {}

TEST_F(ProtoQueryResponseFactoryTest, CreateTransactionsResponse) {}

TEST_F(ProtoQueryResponseFactoryTest, CreateAssetResponse) {}

TEST_F(ProtoQueryResponseFactoryTest, CreateRolesResponse) {}

TEST_F(ProtoQueryResponseFactoryTest, CreateRolePermissionsResponse) {}

TEST_F(ProtoQueryResponseFactoryTest, CreateBlockQueryResponseWithBlock) {}

TEST_F(ProtoQueryResponseFactoryTest, CreateBlockQueryResponseWithError) {}
