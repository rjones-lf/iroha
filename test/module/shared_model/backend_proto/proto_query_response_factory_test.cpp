/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>
#include <boost/optional.hpp>

#include "backend/protobuf/common_objects/proto_common_objects_factory.hpp"
#include "backend/protobuf/proto_query_response_factory.hpp"
#include "cryptography/blob.hpp"
#include "cryptography/crypto_provider/crypto_defaults.hpp"
#include "framework/specified_visitor.hpp"
#include "module/shared_model/builders/protobuf/test_block_builder.hpp"
#include "module/shared_model/builders/protobuf/test_transaction_builder.hpp"
#include "validators/field_validator.hpp"

using namespace shared_model::proto;
using namespace iroha::expected;
using namespace shared_model::interface::types;

using framework::SpecifiedVisitor;
using shared_model::crypto::Blob;
using shared_model::validation::FieldValidator;

class ProtoQueryResponseFactoryTest : public ::testing::Test {
 public:
  std::shared_ptr<ProtoQueryResponseFactory> response_factory =
      std::make_shared<ProtoQueryResponseFactory>();
  std::shared_ptr<ProtoCommonObjectsFactory<FieldValidator>> objects_factory =
      std::make_shared<ProtoCommonObjectsFactory<FieldValidator>>();

  /**
   * Put value of Result<unique_ptr<_>, _> into a shared_ptr
   * @tparam ResultType - type of result value inside a unique_ptr
   * @tparam ErrorType - type of result error
   * @param res - result to be unwrapped
   * @return shared_ptr to result value or to nullptr, if result containts error
   */
  template <typename ResultType, typename ErrorType>
  std::unique_ptr<ResultType> unwrapResult(
      Result<std::unique_ptr<ResultType>, ErrorType> &&res) {
    return res.match(
        [](Value<std::unique_ptr<ResultType>> &val) {
          return std::move(val.value);
        },
        [](const Error<ErrorType> &) -> std::unique_ptr<ResultType> {
          return nullptr;
        });
  }

  void SetUp() override {}
  void TearDown() override {}
};

/**
 * Checks createAccountAssetResponse method of QueryResponseFactory
 * @given collection of account assets
 * @when creating account asset query response via factory
 * @then that response is created @and is well-formed
 */
TEST_F(ProtoQueryResponseFactoryTest, CreateAccountAssetResponse) {
  const HashType kQueryHash{Blob{"my_super_hash"}};

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
    assets.push_back(std::move(asset));
  }
  auto query_response =
      response_factory->createAccountAssetResponse(assets, kQueryHash);

  ASSERT_TRUE(query_response);
  ASSERT_EQ(query_response->queryHash(), kQueryHash);
  ASSERT_NO_THROW({
    const auto &response = boost::apply_visitor(
        SpecifiedVisitor<shared_model::interface::AccountAssetResponse>(),
        query_response->get());
    ASSERT_EQ(response.accountAssets().front().accountId(), kAccountId);
    ASSERT_EQ(response.accountAssets().front().assetId(), kAssetId);
    for (auto i = 1; i < kAccountAssetsNumber; i++) {
      ASSERT_EQ(response.accountAssets()[i - 1].balance(),
                assets[i - 1]->balance());
    }
  });
}

/**
 * Checks createAccountDetailResponse method of QueryResponseFactory
 * @given account details
 * @when creating account detail query response via factory
 * @then that response is created @and is well-formed
 */
TEST_F(ProtoQueryResponseFactoryTest, CreateAccountDetailResponse) {
  const HashType kQueryHash{Blob{"my_super_hash"}};

  const DetailType account_details = "{ fav_meme : doge }";
  auto query_response = response_factory->createAccountDetailResponse(
      account_details, kQueryHash);

  ASSERT_TRUE(query_response);
  ASSERT_EQ(query_response->queryHash(), kQueryHash);
  ASSERT_NO_THROW({
    const auto &response = boost::apply_visitor(
        SpecifiedVisitor<shared_model::interface::AccountDetailResponse>(),
        query_response->get());
    ASSERT_EQ(response.detail(), account_details);
  });
}

/**
 * Checks createAccountResponse method of QueryResponseFactory
 * @given account
 * @when creating account query response via factory
 * @then that response is created @and is well-formed
 */
TEST_F(ProtoQueryResponseFactoryTest, CreateAccountResponse) {
  const HashType kQueryHash{Blob{"my_super_hash"}};

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
  auto query_response = response_factory->createAccountResponse(
      std::move(account), kRoles, kQueryHash);

  ASSERT_TRUE(query_response);
  ASSERT_EQ(query_response->queryHash(), kQueryHash);
  ASSERT_NO_THROW({
    const auto &response = boost::apply_visitor(
        SpecifiedVisitor<shared_model::interface::AccountResponse>(),
        query_response->get());
    ASSERT_EQ(response.account().accountId(), kAccountId);
    ASSERT_EQ(response.account().domainId(), kDomainId);
    ASSERT_EQ(response.account().quorum(), kQuorum);
    ASSERT_EQ(response.account().jsonData(), kJson);
    ASSERT_EQ(response.roles(), kRoles);
  });
}

/**
 * Checks createErrorQueryResponse method of QueryResponseFactory
 * @given
 * @when creating error query responses for a couple of cases via factory
 * @then that response is created @and is well-formed
 */
TEST_F(ProtoQueryResponseFactoryTest, CreateErrorQueryResponse) {
  using ErrorTypes =
      shared_model::interface::QueryResponseFactory::ErrorQueryType;
  const HashType kQueryHash{Blob{"my_super_hash"}};

  const auto kStatelessErrorMsg = "stateless failed";
  const auto kNoSigsErrorMsg = "stateless failed";

  auto stateless_invalid_response = response_factory->createErrorQueryResponse(
      ErrorTypes::kStatelessFailed, kStatelessErrorMsg, kQueryHash);
  auto no_signatories_response = response_factory->createErrorQueryResponse(
      ErrorTypes::kNoSignatories, kNoSigsErrorMsg, kQueryHash);

  ASSERT_TRUE(stateless_invalid_response);
  ASSERT_EQ(stateless_invalid_response->queryHash(), kQueryHash);
  ASSERT_NO_THROW({
    const auto &general_resp = boost::apply_visitor(
        SpecifiedVisitor<shared_model::interface::ErrorQueryResponse>(),
        stateless_invalid_response->get());
    ASSERT_EQ(general_resp.errorMessage(), kStatelessErrorMsg);
    boost::apply_visitor(
        SpecifiedVisitor<
            shared_model::interface::StatelessFailedErrorResponse>(),
        general_resp.get());
  });
  ASSERT_TRUE(no_signatories_response);
  ASSERT_EQ(no_signatories_response->queryHash(), kQueryHash);
  ASSERT_NO_THROW({
    const auto &general_resp = boost::apply_visitor(
        SpecifiedVisitor<shared_model::interface::ErrorQueryResponse>(),
        no_signatories_response->get());
    ASSERT_EQ(general_resp.errorMessage(), kNoSigsErrorMsg);
    boost::apply_visitor(
        SpecifiedVisitor<shared_model::interface::NoSignatoriesErrorResponse>(),
        general_resp.get());
  });
}

/**
 * Checks createSignatoriesResponse method of QueryResponseFactory
 * @given signatories
 * @when creating signatories query response via factory
 * @then that response is created @and is well-formed
 */
TEST_F(ProtoQueryResponseFactoryTest, CreateSignatoriesResponse) {
  const HashType kQueryHash{Blob{"my_super_hash"}};

  const auto pub_key =
      shared_model::crypto::DefaultCryptoAlgorithmType::generateKeypair()
          .publicKey();
  const std::vector<PubkeyType> signatories{pub_key};
  auto query_response =
      response_factory->createSignatoriesResponse(signatories, kQueryHash);

  ASSERT_TRUE(query_response);
  ASSERT_EQ(query_response->queryHash(), kQueryHash);
  ASSERT_NO_THROW({
    const auto &response = boost::apply_visitor(
        SpecifiedVisitor<shared_model::interface::SignatoriesResponse>(),
        query_response->get());
    ASSERT_EQ(response.keys(), signatories);
  });
}

/**
 * Checks createTransactionsResponse method of QueryResponseFactory
 * @given collection of transactions
 * @when creating transactions query response via factory
 * @then that response is created @and is well-formed
 */
TEST_F(ProtoQueryResponseFactoryTest, CreateTransactionsResponse) {
  const HashType kQueryHash{Blob{"my_super_hash"}};

  constexpr int kTransactionsNumber = 5;

  std::vector<std::shared_ptr<shared_model::interface::Transaction>>
      transactions;
  for (auto i = 0; i < kTransactionsNumber; ++i) {
    auto tx =
        TestTransactionBuilder().creatorAccountId(std::to_string(i)).build();
    transactions.push_back(
        std::make_shared<shared_model::proto::Transaction>(std::move(tx)));
  }
  auto query_response =
      response_factory->createTransactionsResponse(transactions, kQueryHash);

  ASSERT_TRUE(query_response);
  ASSERT_EQ(query_response->queryHash(), kQueryHash);
  ASSERT_NO_THROW({
    const auto &response = boost::apply_visitor(
        SpecifiedVisitor<shared_model::interface::TransactionsResponse>(),
        query_response->get());
    for (auto i = 0; i < kTransactionsNumber; ++i) {
      ASSERT_EQ(response.transactions()[i].creatorAccountId(),
                transactions[i]->creatorAccountId());
    }
  });
}

/**
 * Checks createAssetResponse method of QueryResponseFactory
 * @given asset
 * @when creating asset query response via factory
 * @then that response is created @and is well-formed
 */
TEST_F(ProtoQueryResponseFactoryTest, CreateAssetResponse) {
  const HashType kQueryHash{Blob{"my_super_hash"}};

  const AssetIdType kAssetId = "doge#coin";
  const DomainIdType kDomainId = "coin";
  const PrecisionType kPrecision = 2;

  auto asset = unwrapResult(
      objects_factory->createAsset(kAssetId, kDomainId, kPrecision));
  if (not asset) {
    FAIL() << "could not create common object via factory";
  }
  auto query_response =
      response_factory->createAssetResponse(std::move(asset), kQueryHash);

  ASSERT_TRUE(query_response);
  ASSERT_EQ(query_response->queryHash(), kQueryHash);
  ASSERT_NO_THROW({
    const auto &response = boost::apply_visitor(
        SpecifiedVisitor<shared_model::interface::AssetResponse>(),
        query_response->get());
    ASSERT_EQ(response.asset().assetId(), kAssetId);
    ASSERT_EQ(response.asset().domainId(), kDomainId);
    ASSERT_EQ(response.asset().precision(), kPrecision);
  });
}

/**
 * Checks createRolesResponse method of QueryResponseFactory
 * @given collection of roles
 * @when creating roles query response via factory
 * @then that response is created @and is well-formed
 */
TEST_F(ProtoQueryResponseFactoryTest, CreateRolesResponse) {
  const HashType kQueryHash{Blob{"my_super_hash"}};

  const std::vector<RoleIdType> roles{"admin", "user"};
  auto query_response =
      response_factory->createRolesResponse(roles, kQueryHash);

  ASSERT_TRUE(query_response);
  ASSERT_EQ(query_response->queryHash(), kQueryHash);
  ASSERT_NO_THROW({
    const auto &response = boost::apply_visitor(
        SpecifiedVisitor<shared_model::interface::RolesResponse>(),
        query_response->get());
    ASSERT_EQ(response.roles(), roles);
  });
}

/**
 * Checks createRolePermissionsResponse method of QueryResponseFactory
 * @given collection of role permissions
 * @when creating role permissions query response via factory
 * @then that response is created @and is well-formed
 */
TEST_F(ProtoQueryResponseFactoryTest, CreateRolePermissionsResponse) {
  const HashType kQueryHash{Blob{"my_super_hash"}};

  const shared_model::interface::RolePermissionSet perms{
      shared_model::interface::permissions::Role::kGetMyAccount,
      shared_model::interface::permissions::Role::kAddSignatory};
  auto query_response =
      response_factory->createRolePermissionsResponse(perms, kQueryHash);

  ASSERT_TRUE(query_response);
  ASSERT_EQ(query_response->queryHash(), kQueryHash);
  ASSERT_NO_THROW({
    const auto &response = boost::apply_visitor(
        SpecifiedVisitor<shared_model::interface::RolePermissionsResponse>(),
        query_response->get());
    ASSERT_EQ(response.rolePermissions(), perms);
  });
}

/**
 * Checks createBlockQueryResponse method of QueryResponseFactory
 * @given block
 * @when creating block query query response with block via factory
 * @then that response is created @and is well-formed
 */
TEST_F(ProtoQueryResponseFactoryTest, CreateBlockQueryResponseWithBlock) {
  constexpr HeightType kBlockHeight = 42;
  const auto kCreatedTime = iroha::time::now();

  auto block =
      TestBlockBuilder().height(kBlockHeight).createdTime(kCreatedTime).build();
  auto response = response_factory->createBlockQueryResponse(
      std::make_unique<Block>(std::move(block)));

  ASSERT_TRUE(response);
  ASSERT_NO_THROW({
    const auto &block_resp = boost::apply_visitor(
        SpecifiedVisitor<shared_model::interface::BlockResponse>(),
        response->get());
    ASSERT_EQ(block_resp.block().txsNumber(), 0);
    ASSERT_EQ(block_resp.block().height(), kBlockHeight);
    ASSERT_EQ(block_resp.block().createdTime(), kCreatedTime);
  });
}

/**
 * Checks createBlockQueryResponse method of QueryResponseFactory
 * @given error
 * @when creating block query query response with error via factory
 * @then that response is created @and is well-formed
 */
TEST_F(ProtoQueryResponseFactoryTest, CreateBlockQueryResponseWithError) {
  const std::string kErrorMsg = "something's wrong!";
  auto response = response_factory->createBlockQueryResponse(kErrorMsg);

  ASSERT_TRUE(response);
  ASSERT_NO_THROW({
    const auto &error_resp = boost::apply_visitor(
        SpecifiedVisitor<shared_model::interface::BlockErrorResponse>(),
        response->get());
    ASSERT_EQ(error_resp.message(), kErrorMsg);
  });
}
