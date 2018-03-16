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
#include "builders/protobuf/transport_builder.hpp"
#include "module/shared_model/builders/protobuf/test_block_builder.hpp"
#include "module/shared_model/builders/protobuf/test_proposal_builder.hpp"
#include "module/shared_model/builders/protobuf/test_query_builder.hpp"
#include "module/shared_model/builders/protobuf/test_transaction_builder.hpp"

using namespace shared_model;
using namespace shared_model::proto;
using namespace iroha::expected;

class TransportBuilderTest : public ::testing::Test {
 protected:
  void SetUp() override {
    created_time = iroha::time::now();
    account_id = "account@domain";
    quorum = 2;
    counter = 1048576;
    hash = std::string(32, '0');
    height = 1;

    invalid_account_id = "some#invalid?account@@id";
  }

  auto createTransaction() {
    return TestUnsignedTransactionBuilder()
        .createdTime(created_time)
        .txCounter(counter)
        .creatorAccountId(account_id)
        .setAccountQuorum(account_id, quorum)
        .build()
        .signAndAddSignature(keypair);
  }

  auto createInvalidTransaction() {
    return TestUnsignedTransactionBuilder()
        .createdTime(created_time)
        .txCounter(counter)
        .creatorAccountId(invalid_account_id)
        .setAccountQuorum(account_id, quorum)
        .build()
        .signAndAddSignature(keypair);
  }

  auto createQuery() {
    return TestUnsignedQueryBuilder()
        .createdTime(created_time)
        .creatorAccountId(account_id)
        .getAccount(account_id)
        .queryCounter(counter)
        .build()
        .signAndAddSignature(keypair);
  }

  auto createInvalidQuery() {
    return TestUnsignedQueryBuilder()
        .createdTime(created_time)
        .creatorAccountId(invalid_account_id)
        .getAccount(invalid_account_id)
        .queryCounter(counter)
        .build()
        .signAndAddSignature(keypair);
  }

  auto createBlock() {
    return TestBlockBuilder()
        .transactions(std::vector<Transaction>({createTransaction()}))
        .height(1)
        .prevHash(crypto::Hash("asd"))
        .createdTime(created_time)
        .build();
  }

  auto createInvalidBlock() {
    return TestBlockBuilder()
        .transactions(std::vector<Transaction>({createTransaction()}))
        .height(1)
        .prevHash(crypto::Hash("asd"))
        .createdTime(123)  // invalid time
        .build();
  }

  auto createProposal() {
    return TestProposalBuilder()
        .transactions(std::vector<Transaction>({createTransaction()}))
        .height(1)
        .createdTime(created_time)
        .build();
  }

  auto createInvalidProposal() {
    return TestProposalBuilder()
        .transactions(std::vector<Transaction>({createTransaction()}))
        .height(1)
        .createdTime(123)  // invalid time
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
