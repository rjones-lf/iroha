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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <initializer_list>
#include "crypto/keys_manager_impl.hpp"
#include "integration/pipeline/tx_pipeline_integration_test_fixture.hpp"
#include "model/generators/command_generator.hpp"
#include "model/generators/query_generator.hpp"
#include "model/model_crypto_provider_impl.hpp"

const std::chrono::milliseconds kProposalDelay = 1000ms;
const std::chrono::milliseconds kVoteDelay = 1000ms;
const std::chrono::milliseconds kLoadDelay = 1000ms;

class MstPipelineTest : public TxPipelineIntegrationTestFixture {
 public:
  void SetUp() override {
    iroha::ametsuchi::AmetsuchiTest::SetUp();

    auto genesis_tx =
        TransactionGenerator().generateGenesisTransaction(0, {"0.0.0.0:10001"});
    genesis_tx.quorum = 1;
    genesis_block =
        iroha::model::generators::BlockGenerator().generateGenesisBlock(
            0, {genesis_tx});

    manager = std::make_shared<iroha::KeysManagerImpl>("node0");
    auto keypair = manager->loadKeys().value();

    irohad = std::make_shared<TestIrohad>(block_store_path,
                                          pgopt_,
                                          0,
                                          10001,
                                          10,
                                          kProposalDelay,
                                          kVoteDelay,
                                          kLoadDelay,
                                          keypair);

    ASSERT_TRUE(irohad->storage);

    irohad->storage->insertBlock(genesis_block);
    irohad->init();
    irohad->run();
  }

  void TearDown() override {
    iroha::ametsuchi::AmetsuchiTest::TearDown();
    std::remove("node0.pub");
    std::remove("node0.priv");
    std::remove("admin@test.pub");
    std::remove("admin@test.priv");
    std::remove("test@test.pub");
    std::remove("test@test.priv");
    std::remove("multi@test.pub");
    std::remove("multi@test.priv");
  }

  /**
   * Creates txes for multiquorum account and send it
   * @param account is name of new account
   * @param signers is a list of account signers
   * @param quorum is number of signatories required for valid tx
   * @return true if successful
   */
  bool createMultiQuorumAccount(std::string account,
                                std::initializer_list<std::string> signers,
                                uint32_t quorum) {
    CommandGenerator gen;
    auto domain = "@test";
    auto keypair = createNewAccountKeypair(account + domain);
    iroha::KeysManagerImpl manager("admin@test");
    Transaction tx =
        iroha::model::generators::TransactionGenerator().generateTransaction(
            "admin@test",
            1,
            {gen.generateCreateAccount(account, "test", keypair.pubkey),
             gen.generateSetQuorum(account, quorum)});
    for (auto s : signers) {
      auto key = createNewAccountKeypair(s);
      tx.commands.push_back(gen.generateAddSignatory(s, key.pubkey));
    }

    auto admin_keypair = manager.loadKeys().value();
    iroha::model::ModelCryptoProviderImpl(admin_keypair).sign(tx);
    return sendForCommit(tx);
  }

  /**
   * Send model tx, and wait for a commit
   * @param tx is transaction to be sent
   * @param timeout is waiting time for commit
   * @return true if commit happened
   */
  bool sendForCommit(const Transaction &tx,
                     const std::chrono::milliseconds timeout = kProposalDelay
                         + kVoteDelay) {
    std::unique_lock<std::mutex> lk(m);
    irohad->getPeerCommunicationService()->on_commit().subscribe(
        [this](auto) { cv.notify_one(); });
    send(tx);
    return cv.wait_for(lk, timeout) == std::cv_status::no_timeout;
  }

  const std::string account = "multi@test";
  const std::string signer = "signer@signer";
  const std::string asset = "coin#test";

 private:
  /**
   * Send model tx
   * @param tx is transaction to be sent
   */
  void send(const Transaction &tx) {
    auto pb_tx = iroha::model::converters::PbTransactionFactory().serialize(tx);

    google::protobuf::Empty response;
    irohad->getCommandService()->Torii(pb_tx);
  }
};

/**
 * Creates a new signed copy of transaction
 * @param tx is a base transaction
 * @param account is an account for searching the keypair
 */
Transaction sign(const Transaction &tx, const std::string &account) {
  Transaction tmp = tx;
  iroha::KeysManagerImpl manager(account);
  EXPECT_TRUE(manager.loadKeys().has_value());
  auto keypair = manager.loadKeys().value();
  iroha::model::ModelCryptoProviderImpl provider(keypair);
  provider.sign(tmp);
  return tmp;
}

/**
 * @given multisignature account, its signer
 *        AND tx with an AddAssetQuantity command
 * @when sending with author signature and then with signer's one
 * @then firstly there's no commit then it is
 */
TEST_F(MstPipelineTest, OnePeerSendsTest) {
  ASSERT_TRUE(createMultiQuorumAccount("multi", {signer}, 2));
  auto cmd =
      iroha::model::generators::CommandGenerator().generateAddAssetQuantity(
          account, asset, iroha::Amount().createFromString("20.00").value());
  auto tx =
      iroha::model::generators::TransactionGenerator().generateTransaction(
          account, 2, {cmd});
  tx.quorum = 2;

  ASSERT_FALSE(sendForCommit(sign(tx, account)));
  ASSERT_TRUE(sendForCommit(sign(tx, signer)));
}
