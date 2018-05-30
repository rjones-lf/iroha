/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>
#include "acceptance_fixture.hpp"
#include "backend/protobuf/transaction.hpp"
#include "builders/protobuf/queries.hpp"
#include "builders/protobuf/transaction.hpp"
#include "cryptography/crypto_provider/crypto_defaults.hpp"
#include "framework/integration_framework/integration_test_framework.hpp"
#include "utils/query_error_response_visitor.hpp"
#include "validators/permissions.hpp"

using namespace integration_framework;
using namespace shared_model;

class TransferAsset : public AcceptanceFixture {
 public:
  /**
   * Creates the transaction with the user creation commands
   * @param perms are the permissions of the user
   * @return built tx and a hash of its payload
   */
  auto makeUserWithPerms(const std::string &user,
                         const crypto::Keypair &key,
                         const std::vector<std::string> &perms,
                         const std::string &role) {
    return createUserWithPerms(user, key.publicKey(), role, perms)
        .build()
        .signAndAddSignature(kAdminKeypair);
  }

  proto::Transaction addAssets(const std::string &user,
                               const crypto::Keypair &key) {
    return addAssets(user, key, kAmount);
  }

  proto::Transaction addAssets(const std::string &user,
                               const crypto::Keypair &key,
                               const std::string &amount) {
    const std::string kUserId = user + "@test";
    return proto::TransactionBuilder()
        .creatorAccountId(kUserId)
        .createdTime(iroha::time::now())
        .addAssetQuantity(kUserId, kAsset, amount)
        .quorum(1)
        .build()
        .signAndAddSignature(key);
  }

  /**
   * Create valid base pre-build transaction
   * @return pre-build tx
   */
  auto baseTx() {
    return TestUnsignedTransactionBuilder()
        .creatorAccountId(kUser1 + "@test")
        .createdTime(iroha::time::now())
        .quorum(1);
  }

  /**
   * Completes pre-build transaction
   * @param builder is a pre-built tx
   * @return built tx
   */
  template <typename TestTransactionBuilder>
  auto completeTx(TestTransactionBuilder builder) {
    return builder.build().signAndAddSignature(kUser1Keypair);
  }

  const std::string kAmount = "1.0";
  const std::string kDesc = "description";
  const std::string kUser1 = "userone";
  const std::string kUser2 = "usertwo";
  const std::string kRole1 = "roleone";
  const std::string kRole2 = "roletwo";
  const std::string kUser1Id = kUser1 + "@test";
  const std::string kUser2Id = kUser2 + "@test";
  const crypto::Keypair kUser1Keypair =
      crypto::DefaultCryptoAlgorithmType::generateKeypair();
  const crypto::Keypair kUser2Keypair =
      crypto::DefaultCryptoAlgorithmType::generateKeypair();
  const std::vector<std::string> kPerms{
      shared_model::permissions::can_add_asset_qty,
      shared_model::permissions::can_transfer,
      shared_model::permissions::can_receive};
};

/**
 * @given some user with all required permissions
 * @when execute tx with TransferAsset command
 * @then there is the tx in proposal
 */
TEST_F(TransferAsset, Basic) {
  IntegrationTestFramework(4)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms(kUser1, kUser1Keypair, kPerms, kRole1))
      .sendTx(makeUserWithPerms(kUser2, kUser2Keypair, kPerms, kRole2))
      .sendTx(addAssets(kUser1, kUser1Keypair))
      .sendTx(completeTx(
          baseTx().transferAsset(kUser1Id, kUser2Id, kAsset, kDesc, kAmount)))
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 4); })
      .done();
}

/**
 * @given some user with only can_transfer permission
 * @when execute tx with TransferAsset command
 * @then there is an empty proposal
 */
TEST_F(TransferAsset, WithOnlyCanTransferPerm) {
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms(kUser1,
                                kUser1Keypair,
                                {shared_model::permissions::can_transfer},
                                kRole1))
      .skipProposal()
      .skipBlock()
      .sendTx(makeUserWithPerms(kUser2, kUser2Keypair, kPerms, kRole2))
      .skipProposal()
      .skipBlock()
      .sendTx(addAssets(kUser1, kUser1Keypair))
      .skipProposal()
      .skipBlock()
      .sendTx(baseTx()
                  .transferAsset(kUser1Id, kUser2Id, kAsset, kDesc, kAmount)
                  .build()
                  .signAndAddSignature(kUser1Keypair))
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 0); })
      .done();
}

/**
 * @given some user with only can_receive permission
 * @when execute tx with TransferAsset command
 * @then there is an empty proposal
 */
TEST_F(TransferAsset, WithOnlyCanReceivePerm) {
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms(kUser1,
                                kUser1Keypair,
                                {shared_model::permissions::can_receive},
                                kRole1))
      .skipProposal()
      .skipBlock()
      .sendTx(makeUserWithPerms(kUser2, kUser2Keypair, kPerms, kRole2))
      .skipProposal()
      .skipBlock()
      .sendTx(addAssets(kUser1, kUser1Keypair))
      .skipProposal()
      .skipBlock()
      .sendTx(baseTx()
                  .transferAsset(kUser1Id, kUser2Id, kAsset, kDesc, kAmount)
                  .build()
                  .signAndAddSignature(kUser1Keypair))
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 0); })
      .done();
}

/**
 * @given some user with all required permissions
 * @when execute tx with TransferAsset command to nonexistent destination
 * @then there is an empty proposal
 */
TEST_F(TransferAsset, NonexistentDest) {
  std::string nonexistent = "inexist@test";
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms(kUser1, kUser1Keypair, kPerms, kRole1))
      .skipProposal()
      .skipBlock()
      .sendTx(addAssets(kUser1, kUser1Keypair))
      .skipProposal()
      .skipBlock()
      .sendTx(baseTx()
                  .transferAsset(kUser1Id, nonexistent, kAsset, kDesc, kAmount)
                  .build()
                  .signAndAddSignature(kUser1Keypair))
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 0); })
      .done();
}

/**
 * @given pair of users with all required permissions
 * @when execute tx with TransferAsset command with nonexistent asset
 * @then there is an empty proposal
 */
TEST_F(TransferAsset, NonexistentAsset) {
  std::string nonexistent = "inexist#test";
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms(kUser1, kUser1Keypair, kPerms, kRole1))
      .skipProposal()
      .skipBlock()
      .sendTx(makeUserWithPerms(kUser2, kUser2Keypair, kPerms, kRole2))
      .skipProposal()
      .skipBlock()
      .sendTx(addAssets(kUser1, kUser1Keypair))
      .skipProposal()
      .skipBlock()
      .sendTx(
          baseTx()
              .transferAsset(kUser1Id, kUser2Id, nonexistent, kDesc, kAmount)
              .build()
              .signAndAddSignature(kUser1Keypair))
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 0); })
      .done();
}

/**
 * @given pair of users with all required permissions
 * @when execute tx with TransferAsset command with negative amount
 * @then the tx hasn't passed stateless validation
 *       (aka skipProposal throws)
 */
TEST_F(TransferAsset, NegativeAmount) {
  IntegrationTestFramework(3)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms(kUser1, kUser1Keypair, kPerms, kRole1))
      .sendTx(makeUserWithPerms(kUser2, kUser2Keypair, kPerms, kRole2))
      .sendTx(addAssets(kUser1, kUser1Keypair))
      .skipProposal()
      .skipBlock()
      .sendTx(baseTx()
                  .transferAsset(kUser1Id, kUser2Id, kAsset, kDesc, "-1.0")
                  .build()
                  .signAndAddSignature(kUser1Keypair),
              checkStatelessInvalid);
}

/**
 * @given pair of users with all required permissions
 * @when execute tx with TransferAsset command with zero amount
 * @then the tx hasn't passed stateless validation
 *       (aka skipProposal throws)
 */
TEST_F(TransferAsset, ZeroAmount) {
  IntegrationTestFramework(3)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms(kUser1, kUser1Keypair, kPerms, kRole1))
      .sendTx(makeUserWithPerms(kUser2, kUser2Keypair, kPerms, kRole2))
      .sendTx(addAssets(kUser1, kUser1Keypair))
      .skipProposal()
      .skipBlock()
      .sendTx(baseTx()
                  .transferAsset(kUser1Id, kUser2Id, kAsset, kDesc, "0.0")
                  .build()
                  .signAndAddSignature(kUser1Keypair),
              checkStatelessInvalid);
}

/**
 * @given pair of users with all required permissions
 * @when execute tx with TransferAsset command with empty-str description
 * @then it passed to the proposal
 */
TEST_F(TransferAsset, EmptyDesc) {
  IntegrationTestFramework(4)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms(kUser1, kUser1Keypair, kPerms, kRole1))
      .sendTx(makeUserWithPerms(kUser2, kUser2Keypair, kPerms, kRole2))
      .sendTx(addAssets(kUser1, kUser1Keypair))
      .sendTx(completeTx(
          baseTx().transferAsset(kUser1Id, kUser2Id, kAsset, "", kAmount)))
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 4); })
      .done();
}

/**
 * @given pair of users with all required permissions
 * @when execute tx with TransferAsset command with very long description
 * @then the tx hasn't passed stateless validation
 *       (aka skipProposal throws)
 */
TEST_F(TransferAsset, LongDesc) {
  std::string long_desc(100000, 'a');
  auto invalid_tx = completeTx(
      baseTx().transferAsset(kUser1Id, kUser2Id, kAsset, long_desc, kAmount));
  IntegrationTestFramework(3)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms(kUser1, kUser1Keypair, kPerms, kRole1))
      .sendTx(makeUserWithPerms(kUser2, kUser2Keypair, kPerms, kRole2))
      .sendTx(addAssets(kUser1, kUser1Keypair))
      .skipProposal()
      .skipBlock()
      .sendTx(invalid_tx, checkStatelessInvalid)
      .done();
}

/**
 * @given pair of users with all required permissions
 * @when execute tx with TransferAsset command with amount more, than user has
 * @then there is an empty proposal
 */
TEST_F(TransferAsset, MoreThanHas) {
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms(kUser1, kUser1Keypair, kPerms, kRole1))
      .skipProposal()
      .skipBlock()
      .sendTx(makeUserWithPerms(kUser2, kUser2Keypair, kPerms, kRole2))
      .skipProposal()
      .skipBlock()
      .sendTx(addAssets(kUser1, kUser1Keypair, "50.0"))
      .skipProposal()
      .skipBlock()
      .sendTx(completeTx(
          baseTx().transferAsset(kUser1Id, kUser2Id, kAsset, kDesc, "100.0")))
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 0); })
      .done();
}

/**
 * @given pair of users with all required permissions, and tx sender's balance
 * is replenished if required
 * @when execute two txes with TransferAsset command with amount more than a
 * uint256 max half
 * @then first transaction is commited and there is an empty proposal for the
 * second
 */
TEST_F(TransferAsset, Uint256DestOverflow) {
  std::string uint256_halfmax =
      "723700557733226221397318656304299424082937404160253525246609900049457060"
      "2495.0";  // 2**252 - 1
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms(kUser1, kUser1Keypair, kPerms, kRole1))
      .skipProposal()
      .skipBlock()
      .sendTx(makeUserWithPerms(kUser2, kUser2Keypair, kPerms, kRole2))
      .skipProposal()
      .skipBlock()
      .sendTx(addAssets(kUser1, kUser1Keypair, uint256_halfmax))
      .skipProposal()
      .skipBlock()
      // Send first half of the maximum
      .sendTx(completeTx(baseTx().transferAsset(
          kUser1Id, kUser2Id, kAsset, kDesc, uint256_halfmax)))
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); })
      // Restore self balance
      .sendTx(addAssets(kUser1, kUser1Keypair, uint256_halfmax))
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); })
      // Send second half of the maximum
      .sendTx(completeTx(baseTx().transferAsset(
          kUser1Id, kUser2Id, kAsset, kDesc, uint256_halfmax)))
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 0); })
      .done();
}

/**
 * @given some user with all required permissions
 * @when execute tx with TransferAsset command where the source and destination
 * accounts are the same
 * @then the tx hasn't passed stateless validation
 *       (aka skipProposal throws)
 */
TEST_F(TransferAsset, SourceIsDest) {
  IntegrationTestFramework(2)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms(kUser1, kUser1Keypair, kPerms, kRole1))
      .sendTx(addAssets(kUser1, kUser1Keypair))
      .skipProposal()
      .skipBlock()
      .sendTx(completeTx(baseTx().transferAsset(
                  kUser1Id, kUser1Id, kAsset, kDesc, kAmount)),
              checkStatelessInvalid);
}

/**
 * @given some user with all required permission
 * @when execute tx with TransferAsset command where the destination user's
 * domain differ from the source user one
 * @then the tx is commited
 */
TEST_F(TransferAsset, InterDomain) {
  const auto kNewRole = "newrl";
  const auto kNewDomain = "newdom";
  const auto kUser2Id = kUser2 + "@" + kNewDomain;
  IntegrationTestFramework(4)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms(kUser1, kUser1Keypair, kPerms, kRole1))
      .sendTx(
          shared_model::proto::TransactionBuilder()
              .creatorAccountId(
                  integration_framework::IntegrationTestFramework::kAdminId)
              .createdTime(iroha::time::now())
              .createRole(kNewRole,
                          std::vector<std::string>{
                              shared_model::permissions::can_receive})
              .createDomain(kNewDomain, kNewRole)
              .createAccount(
                  kUser2,
                  kNewDomain,
                  crypto::DefaultCryptoAlgorithmType::generateKeypair()
                      .publicKey())
              .createAsset(IntegrationTestFramework::kAssetName, kNewDomain, 1)
              .quorum(1)
              .build()
              .signAndAddSignature(kAdminKeypair))
      .sendTx(addAssets(kUser1, kUser1Keypair, kAmount))
      .sendTx(completeTx(
          baseTx().transferAsset(kUser1Id, kUser2Id, kAsset, kDesc, kAmount)))
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 4); })
      .done();
}
