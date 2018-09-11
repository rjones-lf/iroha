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

#include "builders/protobuf/queries.hpp"
#include "cryptography/crypto_provider/crypto_defaults.hpp"
#include "framework/integration_framework/integration_test_framework.hpp"
#include "framework/specified_visitor.hpp"
#include "integration/acceptance/acceptance_fixture.hpp"

using namespace std::string_literals;
using namespace integration_framework;
using namespace shared_model;

using framework::SpecifiedVisitor;

class MstPipelineTest : public AcceptanceFixture {
 public:
  MstPipelineTest() : mst_itf_{1, {}, [](auto &i) { i.done(); }, true} {}

  /**
   * Sign the transaction
   * @param tx pre-built transaction
   * @param key to sign the transaction
   * @return signed transaction
   */
  template <typename TxBuilder>
  auto signTx(TxBuilder tx, const crypto::Keypair &key) const {
    return tx.build().signAndAddSignature(key).finish();
  }

  /**
   * Creates a mst user
   * @param itf, in which the user will be created
   * @param sigs - number of signatories of that mst user
   * @return itf with created user
   */
  IntegrationTestFramework &makeMstUser(IntegrationTestFramework &itf,
                                        size_t sigs = kSignatories) {
    auto create_user_tx =
        createUserWithPerms(
            kUser,
            kUserKeypair.publicKey(),
            kNewRole,
            {shared_model::interface::permissions::Role::kSetQuorum,
             shared_model::interface::permissions::Role::kAddSignatory,
             shared_model::interface::permissions::Role::kSetDetail})
            .build()
            .signAndAddSignature(kAdminKeypair)
            .finish();
    auto add_signatories_tx = baseTx().quorum(1);
    for (size_t i = 0; i < sigs; ++i) {
      signatories.push_back(
          crypto::DefaultCryptoAlgorithmType::generateKeypair());
      add_signatories_tx =
          add_signatories_tx.addSignatory(kUserId, signatories[i].publicKey());
    }
    add_signatories_tx.setAccountQuorum(kUserId, sigs + 1);
    itf.sendTx(create_user_tx)
        .checkProposal([](auto &proposal) {
          ASSERT_EQ(proposal->transactions().size(), 1);
        })
        .checkVerifiedProposal([](auto &proposal) {
          ASSERT_EQ(proposal->transactions().size(), 1);
        })
        .checkBlock([](auto &proposal) {
          ASSERT_EQ(proposal->transactions().size(), 1);
        })
        .sendTx(add_signatories_tx.build()
                    .signAndAddSignature(kUserKeypair)
                    .finish())
        .checkProposal([](auto &proposal) {
          ASSERT_EQ(proposal->transactions().size(), 1);
        })
        .checkVerifiedProposal([](auto &proposal) {
          ASSERT_EQ(proposal->transactions().size(), 1);
        })
        .checkBlock([](auto &proposal) {
          ASSERT_EQ(proposal->transactions().size(), 1);
        });
    return itf;
  }

  /**
   * Makes a ready-to-send query to get pending transactions
   * @param creator - account, which asks for pending transactions
   * @param key - that account's keypair
   * @return built and signed transaction
   */
  auto makeGetPendingTxsQuery(const std::string &creator,
                              const crypto::Keypair &key) {
    return shared_model::proto::QueryBuilder()
        .createdTime(iroha::time::now())
        .creatorAccountId(creator)
        .queryCounter(1)
        .getPendingTransactions()
        .build()
        .signAndAddSignature(key)
        .finish();
  }

  /**
   * Prepares an instance of ITF with MST turned on
   * @return reference to the MST ITF
   */
  IntegrationTestFramework &prepareMstItf() {
    mst_itf_.setInitialState(kAdminKeypair);
    return makeMstUser(mst_itf_);
  }

  IntegrationTestFramework mst_itf_;

  const std::string kNewRole = "rl"s;
  static const size_t kSignatories = 2;
  std::vector<crypto::Keypair> signatories;
};

/**
 * @given mst account, pair of signers and tx with a SetAccountDetail command
 * @when sending that tx with author signature @and then with signers' ones
 * @then commit appears only after tx is signed by all required signatories
 */
TEST_F(MstPipelineTest, OnePeerSendsTest) {
  auto tx = baseTx()
                .setAccountDetail(kUserId, "fav_meme", "doge")
                .quorum(kSignatories + 1);
  auto checkMstPendingTxStatus =
      [](const shared_model::proto::TransactionResponse &resp) {
        ASSERT_NO_THROW(boost::apply_visitor(
            SpecifiedVisitor<interface::MstPendingResponse>(), resp.get()));
      };
  auto checkEnoughSignaturesCollectedStatus =
      [](const shared_model::proto::TransactionResponse &resp) {
        ASSERT_NO_THROW(boost::apply_visitor(
            SpecifiedVisitor<interface::MstPendingResponse>(), resp.get()));
      };

  auto &mst_itf = prepareMstItf();
  mst_itf.sendTx(signTx(tx, kUserKeypair), checkMstPendingTxStatus)
      .sendTx(signTx(tx, signatories[0]), checkMstPendingTxStatus)
      .sendTx(signTx(tx, signatories[1]), checkEnoughSignaturesCollectedStatus)
      .skipProposal()
      .skipVerifiedProposal()
      .checkBlock([](auto &proposal) {
        ASSERT_EQ(proposal->transactions().size(), 1);
      });
}

/**
 * @given a user that has sent a semi-signed transaction to a ledger
 * @when the user requests pending transactions
 * @then user's semi-signed transaction is returned
 */
TEST_F(MstPipelineTest, GetPendingTxsAwaitingForThisPeer) {
  auto pending_tx = baseTx()
                        .setAccountDetail(kUserId, "fav_meme", "doge")
                        .quorum(kSignatories + 1);

  auto &mst_itf = prepareMstItf();
  auto signed_tx = signTx(pending_tx, kUserKeypair);

  auto pending_tx_check = [pending_hash = signed_tx.hash()](auto &response) {
    ASSERT_NO_THROW({
      const auto &pending_tx_resp = boost::apply_visitor(
          framework::SpecifiedVisitor<
              shared_model::interface::TransactionsResponse>(),
          response.get());
      ASSERT_EQ(pending_tx_resp.transactions().front().hash(), pending_hash);
    });
  };

  // send pending transaction, signing it only with one signatory
  mst_itf.sendTx(signed_tx).sendQuery(
      makeGetPendingTxsQuery(kUserId, kUserKeypair), pending_tx_check);
}

/**
 * @given an empty ledger
 * @when creating pending transactions, which lack two or more signatures, @and
 * signing those transactions with one signature @and executing get pending
 * transactions
 * @then they are returned with initial number of signatures plus one
 */
TEST_F(MstPipelineTest, GetPendingTxsLatestSignatures) {
  auto pending_tx = baseTx()
                        .setAccountDetail(kUserId, "fav_meme", "doge")
                        .quorum(kSignatories + 1);
  auto signatory_check = [](size_t expected_signatures_number) {
    return [expected_signatures_number](auto &response) {
      ASSERT_NO_THROW({
        const auto &pending_tx_resp = boost::apply_visitor(
            framework::SpecifiedVisitor<
                shared_model::interface::TransactionsResponse>(),
            response.get());
        ASSERT_EQ(
            boost::size(pending_tx_resp.transactions().front().signatures()),
            expected_signatures_number);
      });
    };
  };

  auto &mst_itf = prepareMstItf();
  mst_itf.sendTx(signTx(pending_tx, signatories[0]))
      .sendQuery(makeGetPendingTxsQuery(kUserId, kUserKeypair),
                 signatory_check(1))
      .sendTx(signTx(pending_tx, signatories[1]))
      .sendQuery(makeGetPendingTxsQuery(kUserId, kUserKeypair),
                 signatory_check(2));
}

/**
 * @given an empty ledger
 * @when creating pending transactions @and signing them with number of
 * signatures to get over quorum @and executing get pending transactions
 * @then those transactions are not returned
 */
TEST_F(MstPipelineTest, GetPendingTxsNoSignedTxs) {
  auto pending_tx = baseTx()
                        .setAccountDetail(kUserId, "fav_meme", "doge")
                        .quorum(kSignatories + 1);
  auto no_txs_check = [](auto &response) {
    ASSERT_NO_THROW({
      const auto &pending_tx_resp = boost::apply_visitor(
          framework::SpecifiedVisitor<
              shared_model::interface::TransactionsResponse>(),
          response.get());
      ASSERT_TRUE(pending_tx_resp.transactions().empty());
    });
  };

  auto &mst_itf = prepareMstItf();
  mst_itf.sendTx(signTx(pending_tx, signatories[0]))
      .sendTx(signTx(pending_tx, signatories[1]))
      .sendTx(signTx(pending_tx, kUserKeypair))
      .sendQuery(makeGetPendingTxsQuery(kUserId, kUserKeypair), no_txs_check);
}
