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
#include "framework/specified_visitor.hpp"
#include "utils/query_error_response_visitor.hpp"

using namespace integration_framework;
using namespace shared_model;

class TransferAsset : public AcceptanceFixture {
 public:
  /**
   * Creates the transaction with the first user creation commands
   * @param perms are the permissions of the user
   * @return built tx
   */
  auto makeFirstUser(const interface::RolePermissionSet &perms = {
                         interface::permissions::Role::kTransfer}) {
    auto new_perms = perms;
    new_perms.set(interface::permissions::Role::kAddAssetQty);
    const std::string kRole1 = "roleone";
    return AcceptanceFixture::makeUserWithPerms(kRole1, new_perms);
  }

  /**
   * Creates the transaction with the second user creation commands
   * @param perms are the permissions of the user
   * @return built tx
   */
  auto makeSecondUser(const interface::RolePermissionSet &perms = {
                          interface::permissions::Role::kReceive}) {
    return createUserWithPerms(kUser2, kUser2Keypair.publicKey(), kRole2, perms)
        .build()
        .signAndAddSignature(kAdminKeypair)
        .finish();
  }

  proto::Transaction addAssets() {
    return addAssets(kAmount);
  }

  proto::Transaction addAssets(const std::string &amount) {
    return complete(baseTx().addAssetQuantity(kAsset, amount));
  }

  proto::Transaction makeTransfer(const std::string &amount) {
    return complete(
        baseTx().transferAsset(kUserId, kUser2Id, kAsset, kDesc, amount));
  }

  proto::Transaction makeTransfer() {
    return makeTransfer(kAmount);
  }

  const std::string kAmount = "1.0";
  const std::string kDesc = "description";
  const std::string kRole2 = "roletwo";
  const std::string kUser2 = "usertwo";
  const std::string kUser2Id = kUser2 + "@test";
  const crypto::Keypair kUser2Keypair =
      crypto::DefaultCryptoAlgorithmType::generateKeypair();
};

#define check(i) [](auto &block) { ASSERT_EQ(block->transactions().size(), i); }

/**
 * @given pair of users with all required permissions
 * @when execute tx with TransferAsset command
 * @then there is the tx in proposal
 */
TEST_F(TransferAsset, Basic) {
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTxAwait(makeFirstUser(), check(1))
      .sendTxAwait(makeSecondUser(), check(1))
      .sendTxAwait(addAssets(), check(1))
      .sendTxAwait(makeTransfer(), check(1))
      .done();
}

/**
 * @given pair of users
 *        AND the first user without can_transfer permission
 * @when execute tx with TransferAsset command
 * @then there is an empty proposal
 */
TEST_F(TransferAsset, WithoutCanTransfer) {
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTxAwait(makeFirstUser({}), check(1))
      .sendTxAwait(makeSecondUser(), check(1))
      .sendTxAwait(addAssets(), check(1))
      .sendTxAwait(makeTransfer(), check(0))
      .done();
}

/**
 * @given pair of users
 *        AND the second user without can_receive permission
 * @when execute tx with TransferAsset command
 * @then there is an empty proposal
 */
TEST_F(TransferAsset, WithoutCanReceive) {
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTxAwait(makeFirstUser(), check(1))
      // TODO(@l4l) 23/06/18: remove permission with IR-1367
      .sendTxAwait(makeSecondUser({interface::permissions::Role::kAddPeer}),
                   check(1))
      .sendTxAwait(addAssets(), check(1))
      .sendTxAwait(makeTransfer(), check(0))
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
      .sendTxAwait(makeFirstUser(), check(1))
      .sendTxAwait(addAssets(), check(1))
      .sendTxAwait(complete(baseTx().transferAsset(
                       kUserId, nonexistent, kAsset, kDesc, kAmount)),
                   check(0))
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
      .sendTxAwait(makeFirstUser(), check(1))
      .sendTxAwait(makeSecondUser(), check(1))
      .sendTxAwait(addAssets(), check(1))
      .sendTxAwait(complete(baseTx().transferAsset(
                       kUserId, kUser2Id, nonexistent, kDesc, kAmount)),
                   check(0))
      .done();
}

/**
 * @given pair of users with all required permissions
 * @when execute tx with TransferAsset command with negative amount
 * @then the tx hasn't passed stateless validation
 *       (aka skipProposal throws)
 */
TEST_F(TransferAsset, NegativeAmount) {
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTxAwait(makeFirstUser(), check(1))
      .sendTxAwait(makeSecondUser(), check(1))
      .sendTxAwait(addAssets(), check(1))
      .sendTx(makeTransfer("-1.0"), checkStatelessInvalid)
      .done();
}

/**
 * @given pair of users with all required permissions
 * @when execute tx with TransferAsset command with zero amount
 * @then the tx hasn't passed stateless validation
 *       (aka skipProposal throws)
 */
TEST_F(TransferAsset, ZeroAmount) {
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTxAwait(makeFirstUser(), check(1))
      .sendTxAwait(makeSecondUser(), check(1))
      .sendTxAwait(addAssets(), check(1))
      .sendTx(makeTransfer("0.0"), checkStatelessInvalid)
      .done();
}

/**
 * @given pair of users with all required permissions
 * @when execute tx with TransferAsset command with empty-str description
 * @then it passed to the proposal
 */
TEST_F(TransferAsset, EmptyDesc) {
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTxAwait(makeFirstUser(), check(1))
      .sendTxAwait(makeSecondUser(), check(1))
      .sendTxAwait(addAssets(), check(1))
      .sendTxAwait(complete(baseTx().transferAsset(
                       kUserId, kUser2Id, kAsset, "", kAmount)),
                   check(1))
      .done();
}

/**
 * @given pair of users with all required permissions
 * @when execute tx with TransferAsset command with very long description
 * @then the tx hasn't passed stateless validation
 *       (aka skipProposal throws)
 */
TEST_F(TransferAsset, LongDesc) {
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTxAwait(makeFirstUser(), check(1))
      .sendTxAwait(makeSecondUser(), check(1))
      .sendTxAwait(addAssets(), check(1))
      .sendTx(
          complete(baseTx().transferAsset(
              kUserId, kUser2Id, kAsset, std::string(100000, 'a'), kAmount)),
          checkStatelessInvalid)
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
      .sendTxAwait(makeFirstUser(), check(1))
      .sendTxAwait(makeSecondUser(), check(1))
      .sendTxAwait(addAssets("50.0"), check(1))
      .sendTxAwait(makeTransfer("100.0"), check(0))
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
      "578960446186580977117854925043439539266349923328202820197287920039565648"
          "19966.0";  // 2**255 - 2
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTxAwait(makeFirstUser(), check(1))
      .sendTxAwait(makeSecondUser(), check(1))
      .sendTxAwait(addAssets(uint256_halfmax), check(1))
      // Send first half of the maximum
      .sendTxAwait(makeTransfer(uint256_halfmax), check(1))
      // Restore self balance
      .sendTxAwait(addAssets(uint256_halfmax), check(1))
      // Send second half of the maximum
      .sendTxAwait(makeTransfer(uint256_halfmax), check(0))
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
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTxAwait(makeFirstUser(), check(1))
      .sendTxAwait(addAssets(), check(1))
      .sendTx(complete(baseTx().transferAsset(
                  kUserId, kUserId, kAsset, kDesc, kAmount)),
              checkStatelessInvalid);
}

/**
 * @given some user with all required permission
 * @when execute tx with TransferAsset command where the destination user's
 * domain differ from the source user one
 * @then the tx is commited
 */
TEST_F(TransferAsset, InterDomain) {
  const std::string kNewDomain = "newdom";
  const std::string kUser2Id = kUser2 + "@" + kNewDomain;
  const std::string kNewAssetId =
      IntegrationTestFramework::kAssetName + "#" + kNewDomain;

  auto make_second_user =
      baseTx()
          .creatorAccountId(IntegrationTestFramework::kAdminId)
          .createRole(kRole2, {interface::permissions::Role::kReceive})
          .createDomain(kNewDomain, kRole2)
          .createAccount(kUser2, kNewDomain, kUser2Keypair.publicKey())
          .createAsset(IntegrationTestFramework::kAssetName, kNewDomain, 1)
          .build()
          .signAndAddSignature(kAdminKeypair)
          .finish();
  auto add_assets = complete(baseTx().addAssetQuantity(kNewAssetId, kAmount));
  auto make_transfer = complete(
      baseTx().transferAsset(kUserId, kUser2Id, kNewAssetId, kDesc, kAmount));

  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTxAwait(makeFirstUser(), check(1))
      .sendTxAwait(make_second_user, check(1))
      .sendTxAwait(add_assets, check(1))
      .sendTxAwait(make_transfer, check(1))
      .done();
}

/**
 * @given a pair of users with all required permissions
 *        AND asset with big precision
 * @when asset is added and then TransferAsset is called
 * @then txes passed commit and the state as intented
 */
TEST_F(TransferAsset, BigPrecision) {
  const std::string kNewAsset = IntegrationTestFramework::kAssetName + "a";
  const std::string kNewAssetId =
      kNewAsset + "#" + IntegrationTestFramework::kDefaultDomain;
  const auto kPrecision = 5;
  const std::string kInitial = "500";
  const std::string kForTransfer = "1";
  const std::string kLeft = "499";

  auto create_asset =
      baseTx()
          .creatorAccountId(
              integration_framework::IntegrationTestFramework::kAdminId)
          .createAsset(
              kNewAsset, IntegrationTestFramework::kDefaultDomain, kPrecision)
          .build()
          .signAndAddSignature(kAdminKeypair)
          .finish();
  auto add_assets = complete(baseTx().addAssetQuantity(kNewAssetId, kInitial));
  auto make_transfer = complete(baseTx().transferAsset(
      kUserId, kUser2Id, kNewAssetId, kDesc, kForTransfer));

  auto check_balance = [](std::string account_id, std::string val) {
    return [a = std::move(account_id),
            v = val](auto &resp) {
      auto &acc_ast = boost::apply_visitor(
          framework::SpecifiedVisitor<interface::AccountAssetResponse>(),
          resp.get());
      for (auto &ast : acc_ast.accountAssets()) {
        if (ast.accountId() == a) {
          ASSERT_EQ(v, ast.balance().toStringRepr());
        }
      }
    };
  };

  auto make_query = [this](std::string account_id) {
    return baseQry()
        .creatorAccountId(IntegrationTestFramework::kAdminId)
        .getAccountAssets(account_id)
        .build()
        .signAndAddSignature(kAdminKeypair)
        .finish();
  };

  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTxAwait(makeFirstUser(), check(1))
      .sendTxAwait(makeSecondUser(), check(1))
      .sendTxAwait(create_asset, check(1))
      .sendTxAwait(add_assets, check(1))
      .sendTxAwait(make_transfer, check(1))
      .sendQuery(make_query(kUserId), check_balance(kUserId, kLeft))
      .sendQuery(make_query(kUser2Id), check_balance(kUser2Id, kForTransfer))
      .done();
}
