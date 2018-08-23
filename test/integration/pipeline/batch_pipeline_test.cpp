/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>

#include "builders/protobuf/transaction.hpp"
#include "framework/batch_helper.hpp"
#include "framework/integration_framework/integration_test_framework.hpp"
#include "framework/specified_visitor.hpp"
#include "integration/acceptance/acceptance_fixture.hpp"

using namespace shared_model;
using ::testing::ElementsAre;
using ::testing::get;
using ::testing::IsEmpty;
using ::testing::Pointwise;
using ::testing::Truly;
using ::testing::Values;
using ::testing::WithParamInterface;

class BatchPipelineTest
    : public AcceptanceFixture,
      public WithParamInterface<interface::types::BatchType> {
 public:
  auto createFirstUser() {
    return AcceptanceFixture::createUser(kFirstUser,
                                         kFirstUserKeypair.publicKey())
        .build()
        .signAndAddSignature(kAdminKeypair)
        .finish();
  }

  auto createSecondUser(const interface::RolePermissionSet &perms = {
                            interface::permissions::Role::kTransfer,
                            interface::permissions::Role::kAddAssetQty}) {
    return AcceptanceFixture::createUser(kSecondUser,
                                         kSecondUserKeypair.publicKey())
        .build()
        .signAndAddSignature(kAdminKeypair)
        .finish();
  }

  auto createAndAddAssets(std::string account_id,
                          std::string asset_name,
                          std::string amount,
                          const crypto::Keypair &keypair) {
    return proto::TransactionBuilder()
        .creatorAccountId(account_id)
        .quorum(1)
        .createdTime(iroha::time::now())
        .createAsset(asset_name, kDomain, 2)
        .addAssetQuantity(asset_name + "#" + kDomain, amount)
        .build()
        .signAndAddSignature(keypair)
        .finish();
  }

  auto prepareTransferAssetBuilder(const std::string &src_account_id,
                                   const std::string &dest_account_id,
                                   const std::string &asset_name,
                                   const std::string &amount) {
    return TestTransactionBuilder()
        .creatorAccountId(src_account_id)
        .quorum(1)
        .createdTime(iroha::time::now())
        .transferAsset(src_account_id,
                       dest_account_id,
                       asset_name + "#" + kDomain,
                       "",
                       amount);
  }

  auto signedTx(std::shared_ptr<interface::Transaction> tx,
                const crypto::Keypair &keypair) {
    auto signed_blob =
        crypto::DefaultCryptoAlgorithmType::sign(tx->payload(), keypair);
    auto clone_tx = clone(tx.get());
    clone_tx->addSignature(signed_blob, keypair.publicKey());
    return std::shared_ptr<interface::Transaction>(std::move(clone_tx));
  }

 protected:
  std::string createAccountId(const std::string &account_name) {
    return account_name + "@" + kDomain;
  }

  const std::string kRole = "roleone";

  const std::string kAdmin = "admin";
  const std::string kFirstUser = "first";
  const std::string kSecondUser = "second";

  const std::string kFirstUserId = kFirstUser + "@" + kDomain;
  const std::string kSecondUserId = kSecondUser + "@" + kDomain;

  const crypto::Keypair kFirstUserKeypair =
      shared_model::crypto::DefaultCryptoAlgorithmType::generateKeypair();
  const crypto::Keypair kSecondUserKeypair =
      shared_model::crypto::DefaultCryptoAlgorithmType::generateKeypair();

  const std::string kAssetA = "usd";
  const std::string kAssetB = "euro";
};

TEST_P(BatchPipelineTest, ValidBatch) {
  auto batch_transactions = framework::batch::makeTestBatchTransactions(
      GetParam(),
      prepareTransferAssetBuilder(kFirstUserId, kSecondUserId, kAssetA, "1.0"),
      prepareTransferAssetBuilder(kSecondUserId, kFirstUserId, kAssetB, "1.0"));

  auto transaction_sequence_result =
      interface::TransactionSequence::createTransactionSequence(
          interface::types::SharedTxsCollectionType{
              signedTx(batch_transactions[0], kFirstUserKeypair),
              signedTx(batch_transactions[1], kSecondUserKeypair)},
          validation::DefaultUnsignedTransactionsValidator());

  auto transaction_sequence_value =
      framework::expected::val(transaction_sequence_result);
  ASSERT_TRUE(transaction_sequence_value)
      << framework::expected::err(transaction_sequence_result).value().error;

  auto transaction_sequence = transaction_sequence_value.value().value;

  integration_framework::IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTxAwait(createFirstUser(), [&](const auto &) {})
      .sendTxAwait(createSecondUser(), [](const auto &) {})
      .sendTxAwait(
          createAndAddAssets(kFirstUserId, kAssetA, "1.0", kFirstUserKeypair),
          [](const auto &) {})
      .sendTxAwait(
          createAndAddAssets(kSecondUserId, kAssetB, "1.0", kSecondUserKeypair),
          [](const auto &) {})
      .sendTxSequenceAwait(
          transaction_sequence,
          [&transaction_sequence](const auto &block) {
            // check that transactions from block are the same as transactions
            // from transaction sequence
            ASSERT_THAT(block->transactions(),
                        Pointwise(Truly([](const auto &args) {
                                    return get<0>(args) == *get<1>(args);
                                  }),
                                  transaction_sequence.transactions()));
          })
      .done();
}

TEST_F(BatchPipelineTest, InvalidAtomicBatch) {
  auto batch_transactions = framework::batch::makeTestBatchTransactions(
      interface::types::BatchType::ATOMIC,
      prepareTransferAssetBuilder(kFirstUserId, kSecondUserId, kAssetA, "1.0"),
      prepareTransferAssetBuilder(kSecondUserId,
                                  kFirstUserId,
                                  kAssetB,
                                  "2.0")  // invalid tx due to too big transfer
  );

  auto transaction_sequence_result =
      interface::TransactionSequence::createTransactionSequence(
          interface::types::SharedTxsCollectionType{
              signedTx(batch_transactions[0], kFirstUserKeypair),
              signedTx(batch_transactions[1], kSecondUserKeypair)},
          validation::DefaultUnsignedTransactionsValidator());

  auto transaction_sequence_value =
      framework::expected::val(transaction_sequence_result);
  ASSERT_TRUE(transaction_sequence_value)
      << framework::expected::err(transaction_sequence_result).value().error;

  auto transaction_sequence = transaction_sequence_value.value().value;

  integration_framework::IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTxAwait(createFirstUser(), [](const auto &) {})
      .sendTxAwait(createSecondUser(), [](const auto &) {})
      .sendTxAwait(
          createAndAddAssets(kFirstUserId, kAssetA, "1.0", kFirstUserKeypair),
          [](const auto &) {})
      .sendTxAwait(
          createAndAddAssets(kSecondUserId, kAssetB, "1.0", kSecondUserKeypair),
          [](const auto &) {})
      .sendTxSequence(transaction_sequence,
                      [](const auto &statuses) {
                        for (const auto &status : statuses) {
                          EXPECT_NO_THROW(boost::apply_visitor(
                              framework::SpecifiedVisitor<
                                  interface::StatelessValidTxResponse>(),
                              status.get()));
                        }
                      })
      .checkProposal([&transaction_sequence](const auto proposal) {
        ASSERT_THAT(proposal->transactions(),
                    Pointwise(Truly([](const auto &args) {
                                return get<0>(args) == *get<1>(args);
                              }),
                              transaction_sequence.transactions()));
      })
      .checkVerifiedProposal([](const auto verified_proposal) {
        ASSERT_THAT(verified_proposal->transactions(), IsEmpty());
      })
      .done();
}

TEST_F(BatchPipelineTest, InvalidOrderedBatch) {
  auto batch_transactions = framework::batch::makeTestBatchTransactions(
      interface::types::BatchType::ORDERED,
      prepareTransferAssetBuilder(kFirstUserId, kSecondUserId, kAssetA, "0.3"),
      prepareTransferAssetBuilder(kSecondUserId,
                                  kFirstUserId,
                                  kAssetB,
                                  "2.0"),  // invalid tx due to too big transfer
      prepareTransferAssetBuilder(kFirstUserId, kSecondUserId, kAssetA, "0.7"));

  auto transaction_sequence_result =
      interface::TransactionSequence::createTransactionSequence(
          interface::types::SharedTxsCollectionType{
              signedTx(batch_transactions[0], kFirstUserKeypair),
              signedTx(batch_transactions[1], kSecondUserKeypair),
              signedTx(batch_transactions[2], kFirstUserKeypair)},
          validation::DefaultUnsignedTransactionsValidator());

  auto transaction_sequence_value =
      framework::expected::val(transaction_sequence_result);
  ASSERT_TRUE(transaction_sequence_value)
      << framework::expected::err(transaction_sequence_result).value().error;

  auto transaction_sequence = transaction_sequence_value.value().value;

  auto Is = [](const auto tx) {
    return [&tx](const auto &lhs) { return *tx == lhs; };
  };

  integration_framework::IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTxAwait(createFirstUser(), [&](const auto &) {})
      .sendTxAwait(createSecondUser(), [](const auto &) {})
      .sendTxAwait(
          createAndAddAssets(kFirstUserId, kAssetA, "1.0", kFirstUserKeypair),
          [](const auto &) {})
      .sendTxAwait(
          createAndAddAssets(kSecondUserId, kAssetB, "1.0", kSecondUserKeypair),
          [](const auto &) {})
      .sendTxSequenceAwait(transaction_sequence,
                           [&](const auto block) {
                             ASSERT_THAT(
                                 block->transactions(),
                                 ElementsAre(Truly(Is(batch_transactions[0])),
                                             Truly(Is(batch_transactions[2]))));
                           })
      .done();
}

INSTANTIATE_TEST_CASE_P(BatchPipelineParameterizedTest,
                        BatchPipelineTest,
                        // note additional comma is needed to make it compile
                        // https://github.com/google/googletest/issues/1419
                        Values(interface::types::BatchType::ATOMIC,
                               interface::types::BatchType::ORDERED), );
