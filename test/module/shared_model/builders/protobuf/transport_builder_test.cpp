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

#include "block.pb.h"
#include "builders/protobuf/block.hpp"
#include "builders/protobuf/empty_block.hpp"
#include "builders/protobuf/proposal.hpp"
#include "builders/protobuf/queries.hpp"
#include "builders/protobuf/transaction.hpp"
#include "builders/protobuf/transport_builder.hpp"
#include "builders/protobuf/block_variant_transport_builder.hpp"
#include "common/types.hpp"
#include "framework/result_fixture.hpp"
#include "module/shared_model/builders/protobuf/test_block_builder.hpp"
#include "module/shared_model/builders/protobuf/test_empty_block_builder.hpp"
#include "module/shared_model/builders/protobuf/test_proposal_builder.hpp"
#include "module/shared_model/builders/protobuf/test_query_builder.hpp"
#include "module/shared_model/builders/protobuf/test_transaction_builder.hpp"

using namespace shared_model;
using namespace shared_model::proto;
using namespace iroha::expected;
using iroha::operator|;

class TransportBuilderTest : public ::testing::Test {
 protected:
  void SetUp() override {
    created_time = iroha::time::now();
    invalid_created_time = 123;
    account_id = "account@domain";
    quorum = 2;
    counter = 1048576;
    hash = std::string(32, '0');
    height = 1;
    invalid_account_id = "some#invalid?account@@id";
  }

  //-------------------------------------Transaction-------------------------------------
  template <typename TransactionBuilder>
  auto getBaseTransactionBuilder() {
    return TestUnsignedTransactionBuilder()
        .createdTime(created_time)
        .setAccountQuorum(account_id, quorum);
  }

  auto createTransaction() {
    return getBaseTransactionBuilder<shared_model::proto::TransactionBuilder>()
        .creatorAccountId(account_id)
        .build()
        .signAndAddSignature(keypair);
  }

  auto createInvalidTransaction() {
    return getBaseTransactionBuilder<TestTransactionBuilder>()
        .creatorAccountId(invalid_account_id)
        .build()
        .signAndAddSignature(keypair);
  }

  //-------------------------------------Query-------------------------------------
  template <typename QueryBuilder>
  auto getBaseQueryBuilder() {
    return QueryBuilder()
        .createdTime(created_time)
        .getAccount(account_id)
        .queryCounter(counter);
  }

  auto createQuery() {
    return getBaseQueryBuilder<shared_model::proto::QueryBuilder>()
        .creatorAccountId(account_id)
        .build()
        .signAndAddSignature(keypair);
  }

  auto createInvalidQuery() {
    return getBaseQueryBuilder<TestUnsignedQueryBuilder>()
        .creatorAccountId(invalid_account_id)
        .build()
        .signAndAddSignature(keypair);
  }

  //-------------------------------------Block-------------------------------------
  template <typename BlockBuilder>
  auto getBaseBlockBuilder() {
    return BlockBuilder()
        .transactions(std::vector<Transaction>({createTransaction()}))
        .height(1)
        .prevHash(crypto::Hash("asd"));
  }

  auto createBlock() {
    return getBaseBlockBuilder<shared_model::proto::UnsignedBlockBuilder>()
        .createdTime(created_time)
        .build();
  }

  auto createInvalidBlock() {
    return getBaseBlockBuilder<TestBlockBuilder>()
        .createdTime(invalid_created_time)
        .build();
  }

  //-------------------------------------EmptyBlock-------------------------------------
  template <typename EmptyBlockBuilder>
  auto getBaseEmptyBlockBuilder() {
    return EmptyBlockBuilder().height(1).prevHash(crypto::Hash("asd"));
  }

  auto createEmptyBlock() {
    return getBaseEmptyBlockBuilder<
               shared_model::proto::UnsignedEmptyBlockBuilder>()
        .createdTime(created_time)
        .build();
  }

  auto createInvalidEmptyBlock() {
    return getBaseEmptyBlockBuilder<TestEmptyBlockBuilder>()
        .createdTime(invalid_created_time)
        .build();
  }

  //-------------------------------------Proposal-------------------------------------
  template <typename ProposalBuilder>
  auto getBaseProposalBuilder() {
    return ProposalBuilder().createdTime(created_time).height(1);
  }

  auto createProposal() {
    return getBaseProposalBuilder<shared_model::proto::ProposalBuilder>()
        .transactions(std::vector<Transaction>({createTransaction()}))
        .build();
  }

  auto createInvalidProposal() {
    return getBaseProposalBuilder<TestProposalBuilder>()
        .transactions(std::vector<Transaction>({createInvalidTransaction()}))
        .build();
  }

  auto createEmptyProposal() {
    return getBaseProposalBuilder<TestProposalBuilder>()
        .transactions(std::vector<Transaction>())
        .build();
  }

  /**
   * Receives model object, gets transport from it, converts transport into
   * model object and checks if original and obtained model objects are the same
   * @tparam T model object type
   * @tparam SV validator type
   * @param orig_model
   * @param successCase function invoking if value exists
   * @param failCase function invoking when error returned
   */
  template <typename T, typename SV>
  void testTransport(T orig_model,
                     std::function<void(const Value<T> &)> successCase,
                     std::function<void(const Error<std::string> &)> failCase) {
    auto proto_model = orig_model.getTransport();

    auto built_model = TransportBuilder<T, SV>().build(proto_model);

    built_model.match(successCase, failCase);
  }

 protected:
  decltype(iroha::time::now()) created_time;
  decltype(created_time) invalid_created_time;
  std::string account_id;
  uint8_t quorum;
  uint64_t counter;
  std::string hash;
  uint64_t height;

  std::string invalid_account_id;
  shared_model::crypto::Keypair keypair =
      shared_model::crypto::CryptoProviderEd25519Sha3::generateKeypair();
};

//-------------------------------------TRANSACTION-------------------------------------

/**
 * @given valid proto object of transaction
 * @when transport builder constructs model object from it
 * @then original and built objects are equal
 */
TEST_F(TransportBuilderTest, TransactionCreationTest) {
  auto orig_model = createTransaction();
  testTransport<decltype(orig_model),
                validation::DefaultSignableTransactionValidator>(
      orig_model,
      [&orig_model](const Value<decltype(orig_model)> &model) {
        ASSERT_EQ(model.value.getTransport().SerializeAsString(),
                  orig_model.getTransport().SerializeAsString());
      },
      [](const Error<std::string> &msg) {
        std::cout << msg.error << std::endl;
        FAIL();
      });
}

/**
 * @given invalid proto object of transaction
 * @when transport builder constructs model object from it
 * @then error case is executed
 */
TEST_F(TransportBuilderTest, InvalidTransactionCreationTest) {
  auto orig_model = createInvalidTransaction();
  testTransport<decltype(orig_model),
                validation::DefaultSignableTransactionValidator>(
      orig_model,
      [](const Value<decltype(orig_model)>) { FAIL(); },
      [](const Error<std::string> &) { SUCCEED(); });
}

//-------------------------------------QUERY-------------------------------------

/**
 * @given valid proto object of query
 * @when transport builder constructs model object from it
 * @then original and built objects are equal
 */
TEST_F(TransportBuilderTest, QueryCreationTest) {
  auto orig_model = createQuery();
  testTransport<decltype(orig_model),
                validation::DefaultSignableQueryValidator>(
      orig_model,
      [&orig_model](const Value<decltype(orig_model)> &model) {
        ASSERT_EQ(model.value.getTransport().SerializeAsString(),
                  orig_model.getTransport().SerializeAsString());
      },
      [](const Error<std::string> &) { FAIL(); });
}

/**
 * @given invalid proto object of query
 * @when transport builder constructs model object from it
 * @then error case is executed
 */
TEST_F(TransportBuilderTest, InvalidQueryCreationTest) {
  auto orig_model = createInvalidQuery();
  testTransport<decltype(orig_model),
                validation::DefaultSignableQueryValidator>(
      orig_model,
      [](const Value<decltype(orig_model)>) { FAIL(); },
      [](const Error<std::string> &) { SUCCEED(); });
}

//-------------------------------------BLOCK-------------------------------------

/**
 * @given valid proto object of block
 * @when transport builder constructs model object from it
 * @then original and built objects are equal
 */
TEST_F(TransportBuilderTest, BlockCreationTest) {
  auto orig_model = createBlock();
  testTransport<decltype(orig_model), validation::DefaultBlockValidator>(
      orig_model,
      [&orig_model](const Value<decltype(orig_model)> &model) {
        ASSERT_EQ(model.value.getTransport().SerializeAsString(),
                  orig_model.getTransport().SerializeAsString());
      },
      [](const Error<std::string> &) { FAIL(); });
}

/**
 * @given invalid proto object of block
 * @when transport builder constructs model object from it
 * @then error is occured
 */
TEST_F(TransportBuilderTest, InvalidBlockCreationTest) {
  auto orig_model = createInvalidBlock();
  testTransport<decltype(orig_model), validation::DefaultBlockValidator>(
      orig_model,
      [](const Value<decltype(orig_model)>) { FAIL(); },
      [](const Error<std::string> &) { SUCCEED(); });
}

//-------------------------------------PROPOSAL-------------------------------------

/**
 * @given valid proto object of proposal
 * @when transport builder constructs model object from it
 * @then original and built objects are equal
 */
TEST_F(TransportBuilderTest, ProposalCreationTest) {
  auto orig_model = createProposal();
  testTransport<decltype(orig_model), validation::DefaultProposalValidator>(
      orig_model,
      [&orig_model](const Value<decltype(orig_model)> &model) {
        ASSERT_EQ(model.value.getTransport().SerializeAsString(),
                  orig_model.getTransport().SerializeAsString());
      },
      [](const Error<std::string> &) { FAIL(); });
}

/**
 * TODO 21/05/2018 andrei IR-1345 Enable when verified proposal is introduced
 * @given empty proto object of proposal
 * @when transport builder constructs model object from it
 * @then error occurred due to empty transactions
 */
TEST_F(TransportBuilderTest, DISABLED_EmptyProposalCreationTest) {
  auto orig_model = createEmptyProposal();
  testTransport<decltype(orig_model), validation::DefaultProposalValidator>(
      orig_model,
      [](const Value<decltype(orig_model)>) { FAIL(); },
      [](const Error<std::string> &) { SUCCEED(); });
}

//-------------------------------------EmptyBlock-------------------------------------

/**
 * @given valid proto object of empty block
 * @when transport builder constructs model object from it
 * @then original and built objects are equal
 */
TEST_F(TransportBuilderTest, EmptyBlockCreationTest) {
  auto orig_model = createEmptyBlock();
  testTransport<decltype(orig_model), validation::DefaultEmptyBlockValidator>(
      orig_model,
      [&orig_model](const Value<decltype(orig_model)> &model) {
        ASSERT_EQ(model.value.getTransport().SerializeAsString(),
                  orig_model.getTransport().SerializeAsString());
      },
      [](const Error<std::string> &) { FAIL(); });
}

//-------------------------------------BlockVariant-------------------------------------

/**
 * @given Valid block protobuf object with no transactions
 * @when TransportBuilder tries to build BlockVariantType object
 * @then built object contains EmptyBlock shared model object
 * AND it is equal to the original object
 */
TEST_F(TransportBuilderTest, BlockVariantWithValidEmptyBlock) {
  auto emptyBlock = createEmptyBlock();
  interface::BlockVariantType orig_model =
      std::make_shared<decltype(emptyBlock)>(emptyBlock.getTransport());

  auto val = framework::expected::val(
      TransportBuilder<interface::BlockVariantType,
                       validation::DefaultAnyBlockValidator>()
          .build(emptyBlock.getTransport()));
  ASSERT_TRUE(val);
  val | [&emptyBlock](auto &block_variant) {
    iroha::visit_in_place(
        block_variant.value,
        [&emptyBlock](
            const std::shared_ptr<shared_model::interface::EmptyBlock> block) {
          EXPECT_EQ(emptyBlock, *block);
        },
        [](const std::shared_ptr<shared_model::interface::Block>) { FAIL(); });
  };
}

/**
 * @given Invalid block protobuf object with no transactions
 * @when TransportBuilder tries to build BlockVariantType object
 * @then build fails
 */
TEST_F(TransportBuilderTest, BlockVariantWithInvalidEmptyBlock) {
  auto emptyBlock = createInvalidEmptyBlock();

  auto error = framework::expected::err(
      TransportBuilder<interface::BlockVariantType,
                       validation::DefaultAnyBlockValidator>()
          .build(emptyBlock.getTransport()));
  ASSERT_TRUE(error);
}

/**
 * @given Valid block protobuf object with non empty set of transactions
 * @when TransportBuilder tries to build BlockVariantType object
 * @then built object contains Block shared model object
 * AND it is equal to the original object
 */
TEST_F(TransportBuilderTest, BlockVariantWithValidBlock) {
  auto block = createBlock();
  interface::BlockVariantType orig_model =
      std::make_shared<decltype(block)>(block.getTransport());
  auto val = framework::expected::val(
      TransportBuilder<decltype(orig_model),
                       validation::DefaultAnyBlockValidator>()
          .build(block.getTransport()));

  ASSERT_TRUE(val);
  val | [&block](auto &block_variant) {
    iroha::visit_in_place(
        block_variant.value,
        [](std::shared_ptr<shared_model::interface::EmptyBlock>) { FAIL(); },
        [&block](
            std::shared_ptr<shared_model::interface::Block> created_block) {
          EXPECT_EQ(block, *created_block);
        });
  };
}

/**
 * @given Invalid block protobuf object with non-empty transactions set
 * @when TransportBuilder tries to build BlockVariantType object
 * @then build fails
 */
TEST_F(TransportBuilderTest, BlockVariantWithInvalidBlock) {
  auto block = createInvalidBlock();

  auto error = framework::expected::err(
      TransportBuilder<interface::BlockVariantType,
                       validation::DefaultAnyBlockValidator>()
          .build(block.getTransport()));
  ASSERT_TRUE(error);
}
