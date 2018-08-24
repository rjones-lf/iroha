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
  /**
   * Create transaction to create first user
   * @return transaction to create first user
   */
  auto createFirstUser() {
    return AcceptanceFixture::createUser(kFirstUser,
                                         kFirstUserKeypair.publicKey())
        .build()
        .signAndAddSignature(kAdminKeypair)
        .finish();
  }

  /**
   * @return transaction to create second user
   */
  auto createSecondUser() {
    return AcceptanceFixture::createUser(kSecondUser,
                                         kSecondUserKeypair.publicKey())
        .build()
        .signAndAddSignature(kAdminKeypair)
        .finish();
  }

  /**
   * Create transaction to create asset and add its given amount to given user
   * @param account_id account for which amount of asset is added
   * @param asset_name name of the asset to be created and added to the account
   * @param amount amount of the asset to be added to the account
   * @param keypair is used to sign transaction
   * @return transaction with create asset and add asset quantity commands
   */
  auto createAndAddAssets(const interface::types::AccountIdType &account_id,
                          const interface::types::AssetNameType &asset_name,
                          const std::string &amount,
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

  /**
   * Create builder for transaction to do transfer between given users with
   * given amount of given asset
   * @param src_account_id source accound id
   * @param dest_account_id destination account id
   * @param asset_name name of the asset (without domain) to be transferred
   * @param amount amount of asset to be transferred
   * @return transaction builder with transfer asset command
   */
  auto prepareTransferAssetBuilder(
      const interface::types::AccountIdType &src_account_id,
      const interface::types::AccountIdType &dest_account_id,
      const interface::types::AssetNameType &asset_name,
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

  /**
   * Take transaction and sign it with provided signature
   * @param tx to be signed
   * @param keypair to sign
   * @return signed transaction
   */
  auto signedTx(std::shared_ptr<interface::Transaction> tx,
                const crypto::Keypair &keypair) {
    auto signed_blob =
        crypto::DefaultCryptoAlgorithmType::sign(tx->payload(), keypair);
    auto clone_tx = clone(tx.get());
    clone_tx->addSignature(signed_blob, keypair.publicKey());
    return std::shared_ptr<interface::Transaction>(std::move(clone_tx));
  }

  auto createTransactionSequence(
      const interface::types::SharedTxsCollectionType &txs) {
    auto transaction_sequence_result =
        interface::TransactionSequence::createTransactionSequence(
            txs, validation::DefaultUnsignedTransactionsValidator());

    auto transaction_sequence_value =
        framework::expected::val(transaction_sequence_result);
    EXPECT_TRUE(transaction_sequence_value)
        << framework::expected::err(transaction_sequence_result).value().error;

    return transaction_sequence_value.value().value;
  }

 protected:
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

/**
 * Matchers to compare references against pointers
 */

MATCHER(RefAndPointerEq, "") {
  return get<0>(arg) == *get<1>(arg);
}

MATCHER_P(RefAndPointerEq, arg1, "") {
  return arg == *arg1;
}

/**
 * @given any type of batch (ordered or atomic) with two transactions
 * @when transactions are sent to iroha
 * @then both transactions are committed
 */
TEST_P(BatchPipelineTest, ValidBatch) {
  auto batch_transactions = framework::batch::makeTestBatchTransactions(
      GetParam(),
      prepareTransferAssetBuilder(kFirstUserId, kSecondUserId, kAssetA, "1.0"),
      prepareTransferAssetBuilder(kSecondUserId, kFirstUserId, kAssetB, "1.0"));

  SCOPED_TRACE("From valid batch test");
  auto transaction_sequence = createTransactionSequence(
      {signedTx(batch_transactions[0], kFirstUserKeypair),
       signedTx(batch_transactions[1], kSecondUserKeypair)});

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
      .sendTxSequenceAwait(
          transaction_sequence, [&transaction_sequence](const auto &block) {
            // check that transactions from block are the same as transactions
            // from transaction sequence
            ASSERT_THAT(block->transactions(),
                        Pointwise(RefAndPointerEq(),
                                  transaction_sequence.transactions()));
          });
}

/**
 * @given atomic batch of two transactions, with one transaction being stateful
 * invalid
 * @when batch is sent to iroha
 * @then no transaction is committed
 */
TEST_F(BatchPipelineTest, InvalidAtomicBatch) {
  auto batch_transactions = framework::batch::makeTestBatchTransactions(
      interface::types::BatchType::ATOMIC,
      prepareTransferAssetBuilder(kFirstUserId, kSecondUserId, kAssetA, "1.0"),
      prepareTransferAssetBuilder(kSecondUserId,
                                  kFirstUserId,
                                  kAssetB,
                                  "2.0")  // invalid tx due to too big transfer
  );

  SCOPED_TRACE("From invalid atomic batch test");
  auto transaction_sequence = createTransactionSequence(
      {signedTx(batch_transactions[0], kFirstUserKeypair),
       signedTx(batch_transactions[1], kSecondUserKeypair)});

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
        ASSERT_THAT(
            proposal->transactions(),
            Pointwise(RefAndPointerEq(), transaction_sequence.transactions()));
      })
      .checkVerifiedProposal([](const auto verified_proposal) {
        ASSERT_THAT(verified_proposal->transactions(), IsEmpty());
      });
}

/**
 * @given ordered batch of three transactions, with one transaction being
 * stateful invalid
 * @when batch is sent to iroha
 * @then all transactions except stateful invalid one are committed
 */
TEST_F(BatchPipelineTest, InvalidOrderedBatch) {
  auto batch_transactions = framework::batch::makeTestBatchTransactions(
      interface::types::BatchType::ORDERED,
      prepareTransferAssetBuilder(kFirstUserId, kSecondUserId, kAssetA, "0.3"),
      prepareTransferAssetBuilder(
          kSecondUserId,
          kFirstUserId,
          kAssetB,
          "2.0"),  // stateful invalid tx due to too big transfer
      prepareTransferAssetBuilder(kFirstUserId, kSecondUserId, kAssetA, "0.7"));

  SCOPED_TRACE("From InvalidOrderedBatch");
  auto transaction_sequence = createTransactionSequence(
      {signedTx(batch_transactions[0], kFirstUserKeypair),
       signedTx(batch_transactions[1], kSecondUserKeypair),
       signedTx(batch_transactions[2], kFirstUserKeypair)});

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
      .sendTxSequenceAwait(transaction_sequence, [&](const auto block) {
        ASSERT_THAT(
            block->transactions(),
            ElementsAre(
                RefAndPointerEq(transaction_sequence.transactions()[0]),
                RefAndPointerEq(transaction_sequence.transactions()[2])));
      });
}

INSTANTIATE_TEST_CASE_P(BatchPipelineParameterizedTest,
                        BatchPipelineTest,
                        // note additional comma is needed to make it compile
                        // https://github.com/google/googletest/issues/1419
                        Values(interface::types::BatchType::ATOMIC,
                               interface::types::BatchType::ORDERED), );
