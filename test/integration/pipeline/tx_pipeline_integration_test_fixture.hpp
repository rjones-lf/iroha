/**
 * Copyright Soramitsu Co., Ltd. 2017 All Rights Reserved.
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

#ifndef TX_PIPELINE_INTEGRATION_TEST_FIXTURE_HPP
#define TX_PIPELINE_INTEGRATION_TEST_FIXTURE_HPP

#include <atomic>

#include "crypto/keys_manager_impl.hpp"
#include "cryptography/ed25519_sha3_impl/internal/sha3_hash.hpp"
#include "datetime/time.hpp"
#include "framework/test_subscriber.hpp"
#include "main/application.hpp"
#include "main/raw_block_loader.hpp"
#include "model/generators/block_generator.hpp"
#include "module/irohad/ametsuchi/ametsuchi_fixture.hpp"
#include "integration/pipeline/test_irohad.hpp"

// TODO(@warchant): move to separate configuration file
#define MAX_WAITING_TIME 11s

using namespace framework::test_subscriber;
using namespace std::chrono_literals;
using namespace iroha::model::generators;
using iroha::model::Transaction;
using iroha::model::converters::PbTransactionFactory;

class TxPipelineIntegrationTestFixture
    : public iroha::ametsuchi::AmetsuchiTest {
 public:
  TxPipelineIntegrationTestFixture() {
    // spdlog::set_level(spdlog::level::off);
  }

  /**
   * @param transactions in order
   */
  void sendTxsInOrderAndValidate(
      const std::vector<iroha::model::Transaction> &transactions) {
    // test subscribers can't solve duplicate func call.
    ASSERT_FALSE(duplicate_sent.exchange(true));

    // Use one block per one transaction
    const auto num_blocks = transactions.size();

    setTestSubscribers(num_blocks);

    std::for_each(
        transactions.begin(), transactions.end(), [this](auto const &tx) {
          // this-> is needed by gcc
          this->sendTransaction(tx);
          // wait for commit
          std::unique_lock<std::mutex> lk(m);
          cv.wait_for(lk, MAX_WAITING_TIME);
        });

    ASSERT_TRUE(proposal_wrapper->validate());
    ASSERT_EQ(num_blocks, proposals.size());
    ASSERT_EQ(expected_proposals, proposals);

    ASSERT_TRUE(commit_wrapper->validate());
    ASSERT_EQ(num_blocks, blocks.size());
    ASSERT_EQ(expected_blocks, blocks);
  }

  iroha::keypair_t createNewAccountKeypair(
      const std::string &accountName) const {
    auto manager = iroha::KeysManagerImpl(accountName);
    EXPECT_TRUE(manager.createKeys());
    EXPECT_TRUE(manager.loadKeys().has_value());
    return *manager.loadKeys();
  }

  std::shared_ptr<TestIrohad> irohad;

  std::condition_variable cv;
  std::mutex m;

  std::vector<iroha::model::Proposal> proposals;
  std::vector<iroha::model::Block> blocks;

  using Commit = rxcpp::observable<iroha::model::Block>;
  std::unique_ptr<TestSubscriber<iroha::model::Proposal>> proposal_wrapper;
  std::unique_ptr<TestSubscriber<Commit>> commit_wrapper;

  iroha::model::Block genesis_block;
  std::vector<iroha::model::Proposal> expected_proposals;
  std::vector<iroha::model::Block> expected_blocks;

  std::shared_ptr<iroha::KeysManager> manager;

  std::atomic_bool duplicate_sent{false};
  size_t current_height = AmetsuchiTest::FIRST_BLOCK;

 private:
  void setTestSubscribers(size_t num_blocks) {
    // verify proposal
    proposal_wrapper = std::make_unique<TestSubscriber<iroha::model::Proposal>>(
        make_test_subscriber<CallExact>(
            irohad->getPeerCommunicationService()->on_proposal(), num_blocks));
    proposal_wrapper->subscribe(
        [this](auto proposal) { proposals.push_back(proposal); });

    // verify commit and block
    commit_wrapper = std::make_unique<TestSubscriber<Commit>>(
        make_test_subscriber<CallExact>(
            irohad->getPeerCommunicationService()->on_commit(), num_blocks));
    commit_wrapper->subscribe([this](auto commit) {
      commit.subscribe([this](auto block) { blocks.push_back(block); });
    });
    irohad->getPeerCommunicationService()->on_commit().subscribe(
        [this](auto) { cv.notify_one(); });
  }

  void sendTransaction(const iroha::model::Transaction &transaction) {
    // generate expected proposal
    iroha::model::Proposal expected_proposal{{transaction}};

    expected_proposal.height = ++current_height;
    expected_proposals.emplace_back(expected_proposal);

    // generate expected block
    iroha::model::Block expected_block{};
    expected_block.height = expected_proposal.height;
    if(current_height == AmetsuchiTest::FIRST_BLOCK + 1){
      expected_block.prev_hash = genesis_block.hash;
    } else {
      expected_block.prev_hash = expected_blocks.back().hash;
    }
    expected_block.transactions = expected_proposal.transactions;
    expected_block.txs_number = expected_proposal.transactions.size();
    expected_block.created_ts = 0;
    expected_block.hash = iroha::hash(expected_block);

    irohad->getCryptoProvider()->sign(expected_block);

    expected_blocks.push_back(std::move(expected_block));

    // send transactions to torii
    auto pb_tx = PbTransactionFactory().serialize(transaction);

    irohad->getCommandService()->Torii(pb_tx);
  }
};

#endif
