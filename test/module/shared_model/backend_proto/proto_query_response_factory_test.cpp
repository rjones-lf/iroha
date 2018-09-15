/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>
#include <boost/optional.hpp>

#include "backend/protobuf/common_objects/proto_common_objects_factory.hpp"
#include "backend/protobuf/proto_query_response_factory.hpp"
#include "validators/field_validator.hpp"

using namespace shared_model::proto;
using namespace iroha::expected;
using shared_model::validation::FieldValidator;

class ProtoQueryResponseFactoryTest : public ::testing::Test {
 public:
  std::shared_ptr<ProtoQueryResponseFactory> response_factory =
      std::make_shared<ProtoQueryResponseFactory>();
  std::shared_ptr<ProtoCommonObjectsFactory<FieldValidator>> objects_factory =
      std::make_shared<ProtoCommonObjectsFactory<FieldValidator>>();

  /**
   * Put value of Result<unique_ptr, _> into a shared_ptr
   * @tparam ResultType - type of result value inside a unique_ptr
   * @tparam ErrorType - type of result error
   * @param res - result to be unwrapped
   * @return shared_ptr to result value
   */
  template <typename ResultType, typename ErrorType>
  std::shared_ptr<ResultType> unwrapResult(
      Result<std::unique_ptr<ResultType>, ErrorType> &&res) {
    return res.match(
        [](Value<std::unique_ptr<ResultType>> &val) {
          std::shared_ptr<ResultType> ptr = std::move(val.value);
          return ptr;
        },
        [](const Error<ErrorType> &err) -> std::shared_ptr<ResultType> {
          return nullptr;
        });
  }

  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(ProtoQueryResponseFactoryTest, CreateAccountAssetResponse) {
  constexpr int kAccountAssetsNumber = 5;
  const std::string kAccountId = "doge@meme";
  const std::string kAssetId = "dogecoin#iroha";

  std::vector<std::shared_ptr<shared_model::interface::AccountAsset>> assets;
  for (auto i = 1; i < kAccountAssetsNumber; ++i) {
    auto asset = unwrapResult(objects_factory->createAccountAsset(
        kAccountId,
        kAssetId,
        shared_model::interface::Amount(std::to_string(i))));
    if (not asset) {
      FAIL() << "could not create common object via factory";
    }
    assets.push_back(asset);
  }

  auto response = response_factory->createAccountAssetResponse(assets);
  ASSERT_TRUE(response);
  ASSERT_EQ(response->accountAssets().front().accountId(), kAccountId);
  ASSERT_EQ(response->accountAssets().front().assetId(), kAssetId);
  for (auto i = 1; i < kAccountAssetsNumber; i++) {
    ASSERT_EQ(response->accountAssets()[i - 1].balance(),
              assets[i - 1]->balance());
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
