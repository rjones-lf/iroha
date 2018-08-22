/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "framework/integration_framework/integration_test_framework.hpp"
#include "framework/specified_visitor.hpp"
#include "integration/acceptance/acceptance_fixture.hpp"
#include "utils/query_error_response_visitor.hpp"

using namespace shared_model;
using namespace integration_framework;
using namespace shared_model::interface::permissions;

class AccountAssetTxsFixture : public AcceptanceFixture {
 public:
  std::unique_ptr<IntegrationTestFramework> itf_;

  AccountAssetTxsFixture()
      : kSecondDomain("second"),
        kSpectator("spectator"),
        kCloseSpectatorId(kSpectator + "@" + kDomain),
        kRemoteSpectatorId(kSpectator + "@" + kSecondDomain),
        kSpectatorKeypair(
            crypto::DefaultCryptoAlgorithmType::generateKeypair()) {}

  /**
   * Prepare state of ledger:
   * - create accounts of target user, close and remote spectators (close
   *   spectator - another user from the same domain as the domain of target
   *   user account, remote - a user from domain different to domain of target
   *   user account).
   * - execute transfer asset from admin to target account
   * - execute transfer asset from target to admin account
   *
   * @param target_permissions - set of query permissions for target account
   * @param spectator_permissions - set of query permisisons for spectators'
   * accounts
   * @return reference to ITF
   */
  IntegrationTestFramework &prepareState(
      const interface::RolePermissionSet &target_permissions = {},
      const interface::RolePermissionSet &spectator_permissions = {}) {
    auto full_target_permissions = target_permissions;
    full_target_permissions.set(Role::kReceive);
    full_target_permissions.set(Role::kTransfer);
    full_target_permissions.set(Role::kAddAssetQty);
    full_target_permissions.set(Role::kSubtractAssetQty);

    // Add asset to admin and transfer to target account
    auto prepare_tx_1 = complete(
        baseTx(kAdminId)
            .addAssetQuantity(kAssetId, "20000.0")
            .transferAsset(kAdminId, kUserId, kAssetId, "incoming", "500.0"),
        kAdminKeypair);

    // Transfer assets back to admin
    auto prepare_tx_2 = complete(baseTx().transferAsset(
        kUserId, kAdminId, kAssetId, "outgoing", "500.0"));

    // inside prepareState we can use lambda for such assert, since prepare
    // transactions are not going to fail
    auto block_with_tx = [](auto &block) {
      ASSERT_EQ(block->transactions().size(), 1);
    };

    tx_hashes_.push_back(prepare_tx_1.hash());
    tx_hashes_.push_back(prepare_tx_2.hash());
    return itf_
        ->sendTxAwait(makeUserWithPerms(full_target_permissions), block_with_tx)
        .sendTxAwait(
            complete(baseTx(kAdminId)
                         .createRole(kSpectator, spectator_permissions)
                         .createDomain(kSecondDomain, kSpectator)
                         .createAccount(kSpectator,
                                        kSecondDomain,
                                        kSpectatorKeypair.publicKey())
                         .createAccount(
                             kSpectator, kDomain, kSpectatorKeypair.publicKey())
                         .appendRole(kCloseSpectatorId, kSpectator)
                         .detachRole(kCloseSpectatorId,
                                     IntegrationTestFramework::kDefaultRole),
                     kAdminKeypair),
            block_with_tx)
        .sendTxAwait(prepare_tx_1, block_with_tx)
        .sendTxAwait(prepare_tx_2, block_with_tx);
  }

  /**
   * @return a lambda that verifies that query response contains all the hashes
   * of transactions related to the tested pair of account id and asset id
   */
  auto checkAllHashesReceived() {
    return [this](const proto::QueryResponse &response) {
      ASSERT_NO_THROW({
        const auto &resp = boost::apply_visitor(
            framework::SpecifiedVisitor<interface::TransactionsResponse>(),
            response.get());

        const auto &transactions = resp.transactions();
        ASSERT_EQ(boost::size(transactions), tx_hashes_.size());
        auto begin = tx_hashes_.begin();
        auto end = tx_hashes_.end();
        for (const auto &tx : transactions) {
          ASSERT_NE(std::find(begin, end, tx.hash()), end);
        }
      }) << "Actual response: "
         << response.toString();
    };
  }

  /**
   * @return a lambda that verifies that query response says the query was
   * stateful invalid
   */
  auto checkQueryStatefulInvalid() {
    return [](auto &response) {
      ASSERT_TRUE(boost::apply_visitor(
          shared_model::interface::QueryErrorResponseChecker<
              shared_model::interface::StatefulFailedErrorResponse>(),
          response.get()))
          << "Actual response: " << response.toString();
    };
  }

  /**
   * @return a lambda that verifies that query response says the query was
   * stateless invalid
   */
  auto checkQueryStatelessInvalid() {
    return [](auto &response) {
      ASSERT_TRUE(boost::apply_visitor(
          shared_model::interface::QueryErrorResponseChecker<
              shared_model::interface::StatelessFailedErrorResponse>(),
          response.get()))
          << "Actual response: " << response.toString();
    };
  }

 protected:
  void SetUp() override {
    itf_ = std::make_unique<IntegrationTestFramework>(1);
    itf_->setInitialState(kAdminKeypair);
  }

  const interface::types::DomainIdType kSecondDomain;
  const interface::types::AccountNameType kSpectator;
  const interface::types::AccountIdType kCloseSpectatorId;
  const interface::types::AccountIdType kRemoteSpectatorId;
  const crypto::Keypair kSpectatorKeypair;

  std::vector<interface::types::HashType> tx_hashes_;
};

/**
 * C344 Get account transactions from a non-existing account
 * @given a user with kGetAllAccAstTxs permission
 * @when tries to retrieve a list of asset transactions from a non-existing
 * account id
 * @then no transactions are shown. Query should be recognized as stateful
 * invalid
 *
 * TODO igor-egorov, 2018-08-21, IR-1631, wrong response (it returns
 * TransactionsResponse instead of ErrorQueryResponse)
 */
TEST_F(AccountAssetTxsFixture,
       DISABLED_ReadNonExistingAccountHavingAllTxsPermission) {
  const interface::types::AccountIdType non_existing = "nonexisting@" + kDomain;
  prepareState({Role::kGetAllAccAstTxs})
      .sendQuery(complete(baseQry().getAccountAssetTransactions(non_existing,
                                                                kAssetId)),
                 checkQueryStatefulInvalid());
}

/**
 * C345 Pass an empty account id
 * @given a user with kGetAllAccAstTxs permission
 * @when the user tries to retrieve a list of asset transactions from an
 * account with empty id
 * @then the query recognized as stateless invalid
 */
TEST_F(AccountAssetTxsFixture, ReadEmptyAccountHavingAllTxsPermission) {
  const interface::types::AccountIdType empty = "";
  prepareState({Role::kGetAllAccAstTxs})
      .sendQuery(
          complete(baseQry().getAccountAssetTransactions(empty, kAssetId)),
          checkQueryStatelessInvalid());
}

/**
 * C346 Pass an empty asset id
 * @given a user with kGetAllAccAstTxs permission
 * @when the user tries to retrieve a list of own asset transactions specifying
 * empty asset id
 * @then the query recognized as stateless invalid
 */
TEST_F(AccountAssetTxsFixture, ReadEmptyAssetHavingAllTxsPermission) {
  const interface::types::AssetIdType empty = "";
  prepareState({Role::kGetAllAccAstTxs})
      .sendQuery(
          complete(baseQry().getAccountAssetTransactions(kUserId, empty)),
          checkQueryStatelessInvalid());
}

/**
 * C347 Pass a non existing asset id
 * @given a user with kGetAllAccAstTxs permission
 * @when the user tries tor retrieve a list of own asset transactions specifying
 * a non-existing asset id
 * @then the query recognized as stateful invalid
 *
 * TODO igor-egorov, 2018-08-21, IR-1631, wrong response (it returns
 * TransactionsResponse instead of ErrorQueryResponse)
 */
TEST_F(AccountAssetTxsFixture,
       DISABLED_ReadNonExistingAssetHavingAllTxsPermission) {
  const interface::types::AssetIdType non_existing = "nonexisting#" + kDomain;
  prepareState({Role::kGetAllAccAstTxs})
      .sendQuery(complete(baseQry().getAccountAssetTransactions(kUserId,
                                                                non_existing)),
                 checkQueryStatefulInvalid());
}

/**
 * @given a user with kGetAllAccAstTxs permission
 * @when the user tries to retrieve a list of own asset transactions, which
 * contain a transaction with addAssetQuantity command
 * @then all transactions are shown
 *
 * TODO igor-egorov, 2018-08-21, IR-1632, wrong response (response does not
 * contain transaction with addAssetQuantity command)
 */
TEST_F(AccountAssetTxsFixture, DISABLED_OwnTxsIncludingAddAssetQuantity) {
  auto tx = complete(baseTx().addAssetQuantity(kAssetId, "200.0"));
  tx_hashes_.push_back(tx.hash());
  prepareState({Role::kGetAllAccAstTxs})
      .sendTxAwait(
          tx, [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); })
      .sendQuery(
          complete(baseQry().getAccountAssetTransactions(kUserId, kAssetId)),
          checkAllHashesReceived());
}

/**
 * @given a user with kGetAllAccAstTxs permission
 * @when the user tries to retrieve a list of own asset transactions, which
 * contain a transaction with subtractAssetQuantity command
 * @then all transactions are shown
 *
 * TODO igor-egorov, 2018-08-21, IR-1632, wrong response (response does not
 * contain transaction with subtractAssetQuantity command)
 */
TEST_F(AccountAssetTxsFixture, DISABLED_OwnTxsIncludingSubtractAssetQuantity) {
  auto tx = complete(baseTx()
                         .addAssetQuantity(kAssetId, "200.0")
                         .subtractAssetQuantity(kAssetId, "100.0"));
  tx_hashes_.push_back(tx.hash());
  prepareState({Role::kGetAllAccAstTxs})
      .sendTxAwait(
          tx, [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); })
      .sendQuery(
          complete(baseQry().getAccountAssetTransactions(kUserId, kAssetId)),
          checkAllHashesReceived());
}

/*
 * Below are test cases that check different combinations of permission modes
 * (1 - no permissions,   2 - can_get_my_*,
 *  3 - can_get_domain_*, 4 - can_get_all_*)
 * combined with different accounts as query source
 * (1 - an account who was transferring or receiving assets (target account),
 *  2 - an account from the same domain as target account,
 *  3 - an account from domain that different to domain of target account).
 */

/**
 * C348 Get my transactions with a CanGetMyAccountAssetTransactions permission
 * @given a user with kGetMyAccAstTxs permission
 * @when the user tries to retrieve a list of asset transactions with own
 * account id
 * @then all transactions are shown to the user
 */
TEST_F(AccountAssetTxsFixture, OwnTxsWithMyTxsPermission) {
  prepareState({Role::kGetMyAccAstTxs})
      .sendQuery(
          complete(baseQry().getAccountAssetTransactions(kUserId, kAssetId)),
          checkAllHashesReceived());
}

/**
 * C339 Get my transactions without CanGetMyAccountAssetTransactions permission
 * @given a user without any query permission
 * @when the user tries to retrieve a list of asset transactions with own
 * account id
 * @then no transactions are shown. Query should be recognized as stateful
 * invalid
 */
TEST_F(AccountAssetTxsFixture, OwnTxsWithoutAnyPermission) {
  prepareState().sendQuery(
      complete(baseQry().getAccountAssetTransactions(kUserId, kAssetId)),
      checkQueryStatefulInvalid());
}

/**
 * @given a user without any query permission
 * @when the user tries to retrieve a list of asset transactions with account id
 * of a user from the same domain
 * @then no transactions are shown. Query should be recognized as stateful
 * invalid
 */
TEST_F(AccountAssetTxsFixture, AnothersTxsWithoutAnyPermission) {
  prepareState().sendQuery(
      complete(baseQry(kCloseSpectatorId)
                   .getAccountAssetTransactions(kUserId, kAssetId),
               kSpectatorKeypair),
      checkQueryStatefulInvalid());
}

/**
 * @given a user without any query permission
 * @when the user tries to retrieve a list of asset transactions with account id
 * of user from another domain
 * @then no transactions are shown. Query should be recognized as stateful
 * invalid
 */
TEST_F(AccountAssetTxsFixture,
       AnothersTxsFromDifferentDomainWithoutAnyPermission) {
  prepareState().sendQuery(
      complete(baseQry(kRemoteSpectatorId)
                   .getAccountAssetTransactions(kUserId, kAssetId),
               kSpectatorKeypair),
      checkQueryStatefulInvalid());
}

/**
 * C340 Get my transactions with only CanGetDomainAccountAssetTransactions
 * permission
 * @given a user with kGetDomainAccAstTxs permission
 * @when the user tries to retrieve a list of asset transactions with own
 * account id
 * @then all transactions are shown
 */
TEST_F(AccountAssetTxsFixture, OwnTxsWithDomainTxsPermission) {
  prepareState({Role::kGetDomainAccAstTxs})
      .sendQuery(
          complete(baseQry().getAccountAssetTransactions(kUserId, kAssetId)),
          checkAllHashesReceived());
}

/**
 * C341 Get my transactions with only CanGetAllAccountAssetTransactions
 * permission
 * @given a user with kGetAllAccAstTxs permission
 * @when the user tries to retrieve a list of asset transactions with own
 * account id
 * @then all transactions are shown
 */
TEST_F(AccountAssetTxsFixture, OwnTxsWithAllTxsPermission) {
  prepareState({Role::kGetAllAccAstTxs})
      .sendQuery(
          complete(baseQry().getAccountAssetTransactions(kUserId, kAssetId)),
          checkAllHashesReceived());
}

/**
 * Get another's transactions from a different domain with kGetMyAccAstTxs
 * permission
 * @given a user with kGetMyAccAstTxs permission
 * @when the user tries to retrieve a list of asset transactions with account id
 * of a user from another domain
 * @then no transactions are shown. Query should be recognized as stateful
 * invalid
 */
TEST_F(AccountAssetTxsFixture,
       AnothersTxsFromDifferentDomainWithMyTxsPermission) {
  prepareState({}, {Role::kGetMyAccAstTxs})
      .sendQuery(complete(baseQry(kRemoteSpectatorId)
                              .getAccountAssetTransactions(kUserId, kAssetId),
                          kSpectatorKeypair),
                 checkQueryStatefulInvalid());
}

/**
 * Get another's transactions from a different domain with kGetDomainAccAstTxs
 * permission
 * @given a user with kGetDomainAccAstTxs permission
 * @when the user tries to retrieve a list of asset transactions with account id
 * of a user from another domain
 * @then no transactions are shown. Query should be recognized as stateful
 * invalid
 */
TEST_F(AccountAssetTxsFixture,
       AnothersTxsFromDifferentDomainWithDomainTxsPermission) {
  prepareState({}, {Role::kGetDomainAccAstTxs})
      .sendQuery(complete(baseQry(kRemoteSpectatorId)
                              .getAccountAssetTransactions(kUserId, kAssetId),
                          kSpectatorKeypair),
                 checkQueryStatefulInvalid());
}

/**
 * C342 Get account transactions from the domain having
 * CanGetDomainAccountAssetTransactions permission
 * @given a user with kGetDomainAccAstTxs permission
 * @when the user tries to retrieve a list of asset transactions with account id
 * of a user from the same domain
 * @then all transactions are shown
 */
TEST_F(AccountAssetTxsFixture, AnothersTxsWithDomainTxsPermission) {
  prepareState({}, {Role::kGetDomainAccAstTxs})
      .sendQuery(complete(baseQry(kCloseSpectatorId)
                              .getAccountAssetTransactions(kUserId, kAssetId),
                          kSpectatorKeypair),
                 checkAllHashesReceived());
}

/**
 * @given a user with kGetAllAccAstTxs permission
 * @when the user tries to retrieve a list of asset transactions with account id
 * of a user from the same domain
 * @then all transactions are shown
 */
TEST_F(AccountAssetTxsFixture, AnothersTxsWithAllTxsPermission) {
  prepareState({}, {Role::kGetAllAccAstTxs})
      .sendQuery(complete(baseQry(kCloseSpectatorId)
                              .getAccountAssetTransactions(kUserId, kAssetId),
                          kSpectatorKeypair),
                 checkAllHashesReceived());
}

/**
 * @given a user with kGetMyAccAstTxs permission
 * @when the user tries to retrieve a list of asset transactions with account id
 * of a user from the same domain
 * @then no transactions are shown. Query should be recognized as stateful
 * invalid
 */
TEST_F(AccountAssetTxsFixture, AnothersTxsWithMyTxsPermission) {
  prepareState({}, {Role::kGetMyAccAstTxs})
      .sendQuery(complete(baseQry(kCloseSpectatorId)
                              .getAccountAssetTransactions(kUserId, kAssetId),
                          kSpectatorKeypair),
                 checkQueryStatefulInvalid());
}

/**
 * C343 Get account transactions from another domain having
 * CanGetAllAccountAssetTransactions permission
 * @given a user with kGetAllAccAstTxs permission
 * @when the user tries to retrieve a list of asset transactions with account id
 * of a user from another domain
 * @then all transactions are shown
 */
TEST_F(AccountAssetTxsFixture,
       AnothersTxsFromDifferentDomainWithAllTxsPermission) {
  prepareState({}, {Role::kGetAllAccAstTxs})
      .sendQuery(complete(baseQry(kRemoteSpectatorId)
                              .getAccountAssetTransactions(kUserId, kAssetId),
                          kSpectatorKeypair),
                 checkAllHashesReceived());
}
