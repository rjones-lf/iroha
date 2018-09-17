/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>
#include <boost/optional.hpp>

#include "backend/protobuf/common_objects/proto_common_objects_factory.hpp"
#include "backend/protobuf/proto_query_response_factory.hpp"
#include "cryptography/crypto_provider/crypto_defaults.hpp"
#include "framework/specified_visitor.hpp"
#include "module/shared_model/builders/protobuf/test_block_builder.hpp"
#include "module/shared_model/builders/protobuf/test_transaction_builder.hpp"
#include "validators/field_validator.hpp"

using namespace shared_model::proto;
using namespace iroha::expected;
using namespace shared_model::interface::types;

using framework::SpecifiedVisitor;
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

TEST_F(ProtoQueryResponseFactoryTest, CreateAccountDetailResponse) {
  const DetailType account_details = "{ fav_meme : doge }";
  auto response =
      response_factory->createAccountDetailResponse(account_details);

  ASSERT_TRUE(response);
  ASSERT_EQ(response->detail(), account_details);
}

TEST_F(ProtoQueryResponseFactoryTest, CreateAccountResponse) {
  const AccountIdType kAccountId = "doge@meme";
  const DomainIdType kDomainId = "meme";
  const QuorumType kQuorum = 1;
  const JsonType kJson = "{ fav_meme : doge }";
  const std::vector<RoleIdType> kRoles{"admin", "user"};

  auto account = unwrapResult(
      objects_factory->createAccount(kAccountId, kDomainId, kQuorum, kJson));
  if (not account) {
    FAIL() << "could not create common object via factory";
  }
  auto response = response_factory->createAccountResponse(*account, kRoles);

  ASSERT_TRUE(response);
  ASSERT_EQ(response->account().accountId(), kAccountId);
  ASSERT_EQ(response->account().domainId(), kDomainId);
  ASSERT_EQ(response->account().quorum(), kQuorum);
  ASSERT_EQ(response->account().jsonData(), kJson);
  ASSERT_EQ(response->roles(), kRoles);
}

TEST_F(ProtoQueryResponseFactoryTest, CreateErrorQueryResponse) {
  auto response = response_factory->createErrorQueryResponse();

  ASSERT_TRUE(response);
  ASSERT_NO_THROW(boost::apply_visitor(
      SpecifiedVisitor<shared_model::interface::StatelessFailedErrorResponse>(),
      response->get()));
}

TEST_F(ProtoQueryResponseFactoryTest, CreateSignatoriesResponse) {
  const auto pub_key =
      shared_model::crypto::DefaultCryptoAlgorithmType::generateKeypair()
          .publicKey();
  const std::vector<PubkeyType> signatories{pub_key};
  auto response = response_factory->createSignatoriesResponse(signatories);

  ASSERT_TRUE(response);
  ASSERT_EQ(response->keys(), signatories);
}

TEST_F(ProtoQueryResponseFactoryTest, CreateTransactionsResponse) {
  constexpr int kTransactionsNumber = 5;

  std::vector<std::shared_ptr<shared_model::interface::Transaction>>
      transactions;
  for (auto i = 0; i < kTransactionsNumber; ++i) {
    auto tx =
        TestTransactionBuilder().creatorAccountId(std::to_string(i)).build();
    transactions.push_back(
        std::make_shared<shared_model::proto::Transaction>(std::move(tx)));
  }
  auto response = response_factory->createTransactionsResponse(transactions);

  ASSERT_TRUE(response);
  for (auto i = 0; i < kTransactionsNumber; ++i) {
    ASSERT_EQ(response->transactions()[i].creatorAccountId(),
              transactions[i]->creatorAccountId());
  }
}

TEST_F(ProtoQueryResponseFactoryTest, CreateAssetResponse) {
  const AssetIdType kAssetId = "doge#coin";
  const DomainIdType kDomainId = "coin";
  const PrecisionType kPrecision = 2;

  auto asset = unwrapResult(
      objects_factory->createAsset(kAssetId, kDomainId, kPrecision));
  if (not asset) {
    FAIL() << "could not create common object via factory";
  }
  auto response = response_factory->createAssetResponse(*asset);

  ASSERT_TRUE(response);
  ASSERT_EQ(response->asset().assetId(), kAssetId);
  ASSERT_EQ(response->asset().domainId(), kDomainId);
  ASSERT_EQ(response->asset().precision(), kPrecision);
}

TEST_F(ProtoQueryResponseFactoryTest, CreateRolesResponse) {
  const std::vector<RoleIdType> roles{"admin", "user"};
  auto response = response_factory->createRolesResponse(roles);

  ASSERT_TRUE(response);
  ASSERT_EQ(response->roles(), roles);
}

TEST_F(ProtoQueryResponseFactoryTest, CreateRolePermissionsResponse) {
  const shared_model::interface::RolePermissionSet perms{
      shared_model::interface::permissions::Role::kGetMyAccount,
      shared_model::interface::permissions::Role::kAddSignatory};
  auto response = response_factory->createRolePermissionsResponse(perms);

  ASSERT_TRUE(response);
  ASSERT_EQ(response->rolePermissions(), perms);
}

TEST_F(ProtoQueryResponseFactoryTest, CreateBlockQueryResponseWithBlock) {
  constexpr HeightType kBlockHeight = 42;
  const auto kCreatedTime = iroha::time::now();

  auto block =
      TestBlockBuilder().height(kBlockHeight).createdTime(kCreatedTime).build();
  auto response = response_factory->createBlockQueryResponse(block);

  ASSERT_TRUE(response);
  ASSERT_NO_THROW({
    const auto &block_resp = boost::apply_visitor(
        SpecifiedVisitor<shared_model::interface::BlockResponse>(), response->get());
    ASSERT_EQ(block_resp.block().txsNumber(), 0);
    ASSERT_EQ(block_resp.block().height(), kBlockHeight);
    ASSERT_EQ(block_resp.block().createdTime(), kCreatedTime);
  });
}

TEST_F(ProtoQueryResponseFactoryTest, CreateBlockQueryResponseWithError) {
  const std::string kErrorMsg = "something's wrong!";
  auto response = response_factory->createBlockQueryResponse(kErrorMsg);

  ASSERT_TRUE(response);
  ASSERT_NO_THROW({
    const auto &error_resp = boost::apply_visitor(
        SpecifiedVisitor<shared_model::interface::BlockErrorResponse>(), response->get());
    ASSERT_EQ(error_resp.message(), kErrorMsg);
  });
}
