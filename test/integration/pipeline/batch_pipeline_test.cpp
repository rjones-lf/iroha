/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>

#include "builders/protobuf/transaction.hpp"
#include "framework/batch_helper.hpp"
#include "framework/integration_framework/integration_test_framework.hpp"
#include "integration/acceptance/acceptance_fixture.hpp"
#include "module/shared_model/builders/protobuf/test_transaction_builder.hpp"

using namespace shared_model;

class BatchPipelineTest : public AcceptanceFixture {
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

TEST_F(BatchPipelineTest, ValidBatch) {
  auto batch_transactions = framework::batch::makeTestBatchTransactions(
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

  integration_framework::IntegrationTestFramework(2)
      .setInitialState(kAdminKeypair)
      .sendTx(createFirstUser(), [&](const auto &) {})
      .sendTx(createSecondUser(), [](const auto &) {})
      .skipProposal()
      .skipVerifiedProposal()
      .skipBlock()
      .sendTx(
          createAndAddAssets(kFirstUserId, kAssetA, "1.0", kFirstUserKeypair),
          [](const auto &) {})
      .sendTx(
          createAndAddAssets(kSecondUserId, kAssetB, "1.0", kSecondUserKeypair),
          [](const auto &) {})
      .skipProposal()
      .skipVerifiedProposal()
      .skipBlock()
      .sendTxSequenceAwait(transaction_sequence, [](const auto &) {})
      .done();
}
