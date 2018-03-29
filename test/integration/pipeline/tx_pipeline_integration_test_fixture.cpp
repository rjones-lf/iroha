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

#include "integration/pipeline/tx_pipeline_integration_test_fixture.hpp"

#include <algorithm>
#include <atomic>

#include "backend/protobuf/from_old_model.hpp"
#include "cryptography/crypto_provider/crypto_signer.hpp"
#include "model/sha3_hash.hpp"

using namespace framework::test_subscriber;
using namespace std::chrono_literals;
using namespace iroha::model::generators;
using iroha::model::Transaction;

TxPipelineIntegrationTestFixture::TxPipelineIntegrationTestFixture() {
  // spdlog::set_level(spdlog::level::off);
}

/**
 * @param transactions in order
 */
void TxPipelineIntegrationTestFixture::sendTxsInOrderAndValidate(
    const std::vector<iroha::model::Transaction> &transactions) {
  // test subscribers can't solve duplicate func call.
  ASSERT_FALSE(duplicate_sent.exchange(true));

  const auto num_blocks =
      transactions.size();  // Use one block per one transaction
  setTestSubscribers(num_blocks);
  std::for_each(
      transactions.begin(), transactions.end(), [this](auto const &tx) {
        // this-> is needed by gcc
        this->sendTransaction(tx);
        // wait for commit
        std::unique_lock<std::mutex> lk(m);
        cv.wait_for(lk, 20s);
      });
  ASSERT_TRUE(proposal_wrapper->validate());
  ASSERT_EQ(num_blocks, proposals.size());
  // Update proposal timestamp and compare it
  for (auto i = 0u; i < proposals.size(); ++i) {
    expected_proposals[i].created_time = proposals[i]->createdTime();
    auto expected = shared_model::proto::from_old(expected_proposals[i]);
    ASSERT_EQ(expected, *proposals[i]);
  }

  ASSERT_TRUE(commit_wrapper->validate());
  ASSERT_EQ(num_blocks, blocks.size());

  // Update block timestamp and related fields and compare it
  for (auto i = 0u; i < blocks.size(); ++i) {
    auto &expected_block = expected_blocks[i];
    expected_block.created_ts = blocks[i]->createdTime();
    if (i != 0) {
      expected_block.prev_hash = expected_blocks[i - 1].hash;
    }
    expected_block.hash = iroha::hash(expected_block);
    expected_block.sigs = {};

    auto expected = shared_model::proto::from_old(expected_block);
    irohad->getCryptoSigner()->sign(expected);
    expected_block =
        *std::unique_ptr<iroha::model::Block>(expected.makeOldModel());

    // compare old and new model object by their hash
    ASSERT_EQ(expected_block.hash.to_hexstring(), blocks[i]->hash().hex());
  }
}

iroha::keypair_t TxPipelineIntegrationTestFixture::createNewAccountKeypair(
    const std::string &accountName) const {
  auto manager = iroha::KeysManagerImpl(accountName);
  EXPECT_TRUE(manager.createKeys());
  EXPECT_TRUE(manager.loadKeys());
  return *manager.loadKeys();
}

void TxPipelineIntegrationTestFixture::setTestSubscribers(size_t num_blocks) {
  // verify proposal
  proposal_wrapper = std::make_unique<
      TestSubscriber<std::shared_ptr<shared_model::interface::Proposal>>>(
      make_test_subscriber<CallExact>(
          irohad->getPeerCommunicationService()->on_proposal(), num_blocks));
  proposal_wrapper->subscribe(
      [this](auto proposal) { proposals.push_back(proposal); });

  // verify commit and block
  commit_wrapper = std::make_unique<TestSubscriber<iroha::Commit>>(
      make_test_subscriber<CallExact>(
          irohad->getPeerCommunicationService()->on_commit(), num_blocks));
  commit_wrapper->subscribe([this](auto commit) {
    commit.subscribe([this](auto block) { blocks.push_back(block); });
  });
  irohad->getPeerCommunicationService()->on_commit().subscribe(
      [this](auto) { cv.notify_one(); });
}

void TxPipelineIntegrationTestFixture::sendTransaction(
    const iroha::model::Transaction &transaction) {
  // generate expected proposal
  iroha::model::Proposal expected_proposal{
      std::vector<Transaction>{transaction}};
  expected_proposal.height = next_height_count++;
  expected_proposals.emplace_back(expected_proposal);

  // generate expected block
  iroha::model::Block expected_block = iroha::model::Block{};
  expected_block.height = expected_proposal.height;
  expected_block.prev_hash =
      next_height_count == 3 ? genesis_block.hash : expected_blocks.back().hash;
  expected_block.transactions = expected_proposal.transactions;
  expected_block.txs_number = expected_proposal.transactions.size();
  expected_block.created_ts = 0;
  expected_block.hash = iroha::hash(expected_block);

  auto expected = shared_model::proto::from_old(expected_block);
  irohad->getCryptoSigner()->sign(expected);
  expected_block =
      *std::unique_ptr<iroha::model::Block>(expected.makeOldModel());

  expected_blocks.emplace_back(expected_block);

  // send transactions to torii
  auto pb_tx =
      iroha::model::converters::PbTransactionFactory().serialize(transaction);

  irohad->getCommandService()->Torii(pb_tx);
}
