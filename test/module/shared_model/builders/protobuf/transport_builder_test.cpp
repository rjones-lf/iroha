/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>

#include <boost/range/irange.hpp>
#include "builders/protobuf/block.hpp"
#include "builders/protobuf/block_variant_transport_builder.hpp"
#include "builders/protobuf/empty_block.hpp"
#include "builders/protobuf/proposal.hpp"
#include "builders/protobuf/queries.hpp"
#include "builders/protobuf/transaction.hpp"
#include "builders/protobuf/transaction_sequence_builder.hpp"
#include "builders/protobuf/transport_builder.hpp"
#include "common/types.hpp"
#include "endpoint.pb.h"
#include "framework/batch_helper.hpp"
#include "framework/result_fixture.hpp"
#include "interfaces/common_objects/types.hpp"
#include "interfaces/iroha_internal/transaction_sequence.hpp"
#include "module/shared_model/builders/protobuf/test_block_builder.hpp"
#include "module/shared_model/builders/protobuf/test_empty_block_builder.hpp"
#include "module/shared_model/builders/protobuf/test_proposal_builder.hpp"
#include "module/shared_model/builders/protobuf/test_query_builder.hpp"
#include "module/shared_model/builders/protobuf/test_transaction_builder.hpp"
#include "validators/transactions_collection/batch_order_validator.hpp"

using namespace shared_model;
using namespace shared_model::proto;
using namespace iroha::expected;
using iroha::operator|;

using TransactionSequenceBuilder = TransportBuilder<
    interface::TransactionSequence,
    validation::TransactionsCollectionValidator<
        validation::TransactionValidator<
            validation::FieldValidator,
            validation::CommandValidatorVisitor<validation::FieldValidator>>>>;

class TransportBuilderTest : public ::testing::Test {
 protected:
  void SetUp() override {
    created_time = iroha::time::now();
    invalid_created_time = 123;
    account_id = "account@domain";
    account_id2 = "acccount@domain";
    quorum = 2;
    counter = 1048576;
    hash = shared_model::crypto::Hash(std::string(32, '0'));
    invalid_hash = shared_model::crypto::Hash("");
    height = 1;
    invalid_account_id = "some#invalid?account@@id";
  }

  //-------------------------------------Transaction-------------------------------------
  template <typename TransactionBuilder>
  auto getBaseTransactionBuilder() {
    return TestUnsignedTransactionBuilder()
        .createdTime(created_time)
        .quorum(quorum)
        .setAccountQuorum(account_id, quorum);
  }
  auto createUnbuildTransaction() {
    return getBaseTransactionBuilder<shared_model::proto::TransactionBuilder>()
        .creatorAccountId(account_id);
  }
  auto createTransaction() {
    return getBaseTransactionBuilder<shared_model::proto::TransactionBuilder>()
        .creatorAccountId(account_id)
        .build()
        .signAndAddSignature(keypair)
        .finish();
  }

  auto createInvalidTransaction() {
    return getBaseTransactionBuilder<TestTransactionBuilder>()
        .creatorAccountId(invalid_account_id)
        .build()
        .signAndAddSignature(keypair)
        .finish();
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
        .signAndAddSignature(keypair)
        .finish();
  }

  auto createInvalidQuery() {
    return getBaseQueryBuilder<TestUnsignedQueryBuilder>()
        .creatorAccountId(invalid_account_id)
        .build()
        .signAndAddSignature(keypair)
        .finish();
  }

  //-------------------------------------Block-------------------------------------
  template <typename BlockBuilder>
  auto getBaseBlockBuilder() {
    return BlockBuilder()
        .transactions(std::vector<Transaction>({createTransaction()}))
        .height(1)
        .createdTime(created_time);
  }

  auto createBlock() {
    return getBaseBlockBuilder<shared_model::proto::BlockBuilder>()
        .prevHash(hash)
        .build()
        .signAndAddSignature(keypair)
        .finish();
  }

  auto createInvalidBlock() {
    return getBaseBlockBuilder<TestBlockBuilder>()
        .prevHash(invalid_hash)
        .build();
  }

  //-------------------------------------EmptyBlock-------------------------------------
  template <typename EmptyBlockBuilder>
  auto getBaseEmptyBlockBuilder() {
    return EmptyBlockBuilder().height(1).createdTime(created_time);
  }

  auto createEmptyBlock() {
    return getBaseEmptyBlockBuilder<
               shared_model::proto::UnsignedEmptyBlockBuilder>()
        .prevHash(hash)
        .build();
  }

  auto createInvalidEmptyBlock() {
    return getBaseEmptyBlockBuilder<TestEmptyBlockBuilder>()
        .prevHash(invalid_hash)
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
   * @tparam ObjectOriginalModel - model object type
   * @tparam Validator - validator type
   * @param orig_model
   * @param successCase function invoking if value exists
   * @param failCase function invoking when error returned
   */
  template <typename Validator,
            typename ObjectOriginalModel,
            typename SuccessCase,
            typename FailCase>
  void testTransport(const ObjectOriginalModel &orig_model,
                     SuccessCase &&successCase,
                     FailCase &&failCase) {
    auto proto_model = orig_model.getTransport();

    auto built_model =
        TransportBuilder<ObjectOriginalModel, Validator>().build(proto_model);

    built_model.match(successCase, failCase);
  }

 protected:
  decltype(iroha::time::now()) created_time;
  decltype(created_time) invalid_created_time;
  std::string account_id;
  std::string account_id2;
  uint8_t quorum;
  uint64_t counter;
  shared_model::crypto::Hash hash;
  shared_model::crypto::Hash invalid_hash;
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
  testTransport<validation::DefaultSignedTransactionValidator>(
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
  testTransport<validation::DefaultSignedTransactionValidator>(
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
  testTransport<validation::DefaultSignedQueryValidator>(
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
  testTransport<validation::DefaultSignedQueryValidator>(
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
  testTransport<validation::DefaultUnsignedBlockValidator>(
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
  testTransport<validation::DefaultUnsignedBlockValidator>(
      orig_model,
      [](const Value<std::decay_t<decltype(orig_model)>> &) { FAIL(); },
      [](const Error<const std::string> &) { SUCCEED(); });
}

//-------------------------------------PROPOSAL-------------------------------------

/**
 * @given valid proto object of proposal
 * @when transport builder constructs model object from it
 * @then original and built objects are equal
 */
TEST_F(TransportBuilderTest, ProposalCreationTest) {
  auto orig_model = createProposal();
  testTransport<validation::DefaultProposalValidator>(
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
  testTransport<validation::DefaultProposalValidator>(
      orig_model,
      [](const Value<decltype(orig_model)> &) { FAIL(); },
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
  testTransport<validation::DefaultEmptyBlockValidator>(
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
 * @when TransportBuilder tries to build BlockVariant object
 * @then built object contains EmptyBlock shared model object
 * AND it is equal to the original object
 */
TEST_F(TransportBuilderTest, BlockVariantWithValidEmptyBlock) {
  auto emptyBlock = createEmptyBlock();
  interface::BlockVariant orig_model =
      std::make_shared<decltype(emptyBlock)>(emptyBlock.getTransport());

  auto val = framework::expected::val(
      TransportBuilder<interface::BlockVariant,
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
 * @when TransportBuilder tries to build BlockVariant object
 * @then build fails
 */
TEST_F(TransportBuilderTest, BlockVariantWithInvalidEmptyBlock) {
  auto emptyBlock = createInvalidEmptyBlock();

  auto error = framework::expected::err(
      TransportBuilder<interface::BlockVariant,
                       validation::DefaultAnyBlockValidator>()
          .build(emptyBlock.getTransport()));
  ASSERT_TRUE(error);
}

/**
 * @given Valid block protobuf object with non empty set of transactions
 * @when TransportBuilder tries to build BlockVariant object
 * @then built object contains Block shared model object
 * AND it is equal to the original object
 */
TEST_F(TransportBuilderTest, BlockVariantWithValidBlock) {
  auto block = createBlock();
  interface::BlockVariant orig_model =
      std::make_shared<decltype(block)>(block.getTransport());
  auto built_block = TransportBuilder<decltype(orig_model),
                                      validation::DefaultAnyBlockValidator>()
                         .build(block.getTransport());
  auto val = framework::expected::val(built_block);

  ASSERT_TRUE(val) << framework::expected::err(built_block).value().error;
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
 * @when TransportBuilder tries to build BlockVariant object
 * @then build fails
 */
TEST_F(TransportBuilderTest, BlockVariantWithInvalidBlock) {
  auto block = createInvalidBlock();

  auto error = framework::expected::err(
      TransportBuilder<interface::BlockVariant,
                       validation::DefaultAnyBlockValidator>()
          .build(block.getTransport()));
  ASSERT_TRUE(error);
}

//---------------------------Transaction Sequence-------------------------------

/**
 * @given empty range of transactions
 * @when TransportBuilder tries to build TransactionSequence object
 * @then built object contains TransactionSequence shared model object
 * AND object will not created
 */
TEST_F(TransportBuilderTest, TransactionSequenceEmpty) {
  iroha::protocol::TxList tx_list;
  auto val =
      framework::expected::val(TransactionSequenceBuilder().build(tx_list));
  ASSERT_FALSE(val);
}

struct getProtocolTx {
  iroha::protocol::Transaction operator()(
      const std::shared_ptr<interface::Transaction> tx) const {
    return std::static_pointer_cast<proto::Transaction>(tx)->getTransport();
  }
};

/**
 * @given sequence of transaction with a right order
 * @when TransportBuilder tries to build TransactionSequence object
 * @then  built object contains TransactionSequence shared model object
 */
TEST_F(TransportBuilderTest, TransactionSequenceCorrect) {
  iroha::protocol::TxList tx_list;
  auto now = iroha::time::now();
  auto batch1 = framework::batch::createUnsignedBatchTransactions(
      interface::types::BatchType::ATOMIC, 10, now);
  auto batch2 = framework::batch::createUnsignedBatchTransactions(
      interface::types::BatchType::ATOMIC, 5, now + 1);
  auto batch3 = framework::batch::createUnsignedBatchTransactions(
      interface::types::BatchType::ATOMIC, 5, now + 2);
  std::for_each(std::begin(batch1), std::end(batch1), [&tx_list](auto &tx) {
    new (tx_list.add_transactions()) iroha::protocol::Transaction(
        std::static_pointer_cast<proto::Transaction>(tx)->getTransport());
  });

  std::for_each(std::begin(batch2), std::end(batch2), [&tx_list](auto &tx) {
    new (tx_list.add_transactions()) iroha::protocol::Transaction(
        std::static_pointer_cast<proto::Transaction>(tx)->getTransport());
  });

  new (tx_list.add_transactions())
      iroha::protocol::Transaction(createTransaction().getTransport());
  new (tx_list.add_transactions())
      iroha::protocol::Transaction(createTransaction().getTransport());
  new (tx_list.add_transactions())
      iroha::protocol::Transaction(createTransaction().getTransport());
  std::for_each(std::begin(batch3), std::end(batch3), [&tx_list](auto &tx) {
    new (tx_list.add_transactions()) iroha::protocol::Transaction(
        std::static_pointer_cast<proto::Transaction>(tx)->getTransport());
  });
  new (tx_list.add_transactions())
      iroha::protocol::Transaction(createTransaction().getTransport());

  auto val =
      framework::expected::val(TransactionSequenceBuilder().build(tx_list));

  val | [](auto &seq) { EXPECT_EQ(boost::size(seq.value.transactions()), 24); };
}
/**
 * @given batch of transaction with transaction in the middle
 * @when TransportBuilder tries to build TransactionSequence object
 * @then  built an error
 * @note disabled because current algorithm of creating batches can process list
 * of transaction where in the middle of the batch can appear single independent
 * transaction
 */
TEST_F(TransportBuilderTest, DISABLED_TransactionInteraptedBatch) {
  iroha::protocol::TxList tx_list;
  auto batch = framework::batch::createUnsignedBatchTransactions(
      interface::types::BatchType::ATOMIC, 10);
  std::for_each(std::begin(batch), std::begin(batch) + 3, [&tx_list](auto &tx) {
    new (tx_list.add_transactions()) iroha::protocol::Transaction(
        std::static_pointer_cast<proto::Transaction>(tx)->getTransport());
  });
  new (tx_list.add_transactions())
      iroha::protocol::Transaction(createTransaction().getTransport());
  std::for_each(std::begin(batch) + 3, std::end(batch), [&tx_list](auto &tx) {
    new (tx_list.add_transactions()) iroha::protocol::Transaction(
        std::static_pointer_cast<proto::Transaction>(tx)->getTransport());
  });

  auto error =
      framework::expected::err(TransactionSequenceBuilder().build(tx_list));
  ASSERT_TRUE(error);
}

/**
 * @given batch of transaction with wrong order
 * @when TransportBuilder tries to build TransactionSequence object
 * @then  built an error
 */
TEST_F(TransportBuilderTest, BatchWrongOrder) {
  iroha::protocol::TxList tx_list;
  auto batch = framework::batch::createUnsignedBatchTransactions(
      interface::types::BatchType::ATOMIC, 10);
  std::for_each(std::begin(batch) + 3, std::end(batch), [&tx_list](auto &tx) {
    new (tx_list.add_transactions()) iroha::protocol::Transaction(
        std::static_pointer_cast<proto::Transaction>(tx)->getTransport());
  });
  std::for_each(std::begin(batch), std::begin(batch) + 3, [&tx_list](auto &tx) {
    new (tx_list.add_transactions()) iroha::protocol::Transaction(
        std::static_pointer_cast<proto::Transaction>(tx)->getTransport());
  });
  auto error =
      framework::expected::err(TransactionSequenceBuilder().build(tx_list));
  ASSERT_TRUE(error);
}
