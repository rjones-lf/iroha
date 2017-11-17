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

#include "crypto/hash.hpp"
#include "crypto/keys_manager_impl.hpp"
#include "datetime/time.hpp"
#include "framework/test_subscriber.hpp"
#include "main/application.hpp"
#include "main/raw_block_insertion.hpp"
#include "model/generators/block_generator.hpp"
#include "module/irohad/ametsuchi/ametsuchi_fixture.hpp"

using namespace framework::test_subscriber;
using namespace std::chrono_literals;
using namespace iroha::model::generators;
using iroha::model::Transaction;

class TestIrohad : public Irohad {
 public:
  TestIrohad(const std::string &block_store_dir,
             const std::string &redis_host,
             size_t redis_port,
             const std::string &pg_conn,
             size_t torii_port,
             size_t internal_port,
             size_t max_proposal_size,
             std::chrono::milliseconds proposal_delay,
             std::chrono::milliseconds vote_delay,
             std::chrono::milliseconds load_delay,
             const iroha::keypair_t &keypair)
      : Irohad(block_store_dir,
               redis_host,
               redis_port,
               pg_conn,
               torii_port,
               internal_port,
               max_proposal_size,
               proposal_delay,
               vote_delay,
               load_delay,
               keypair) {}

  auto &getCommandService() {
    return command_service;
  }

  auto &getPeerCommunicationService() {
    return pcs;
  }

  auto &getCryptoProvider() {
    return crypto_verifier;
  }

  void run() override {
    grpc::ServerBuilder builder;
    int port = 0;
    builder.AddListeningPort("0.0.0.0:" + std::to_string(internal_port_),
                             grpc::InsecureServerCredentials(),
                             &port);
    builder.RegisterService(ordering_init.ordering_gate_transport.get());
    builder.RegisterService(ordering_init.ordering_service_transport.get());
    builder.RegisterService(yac_init.consensus_network.get());
    builder.RegisterService(loader_init.service.get());
    internal_server = builder.BuildAndStart();
    internal_thread = std::thread([this] { internal_server->Wait(); });
    log_->info("===> iroha initialized");
  }
};

class TxPipelineIntegrationTestFixture
    : public iroha::ametsuchi::AmetsuchiTest {
 public:
  TxPipelineIntegrationTestFixture() {
    // spdlog::set_level(spdlog::level::off);
  }

  /**
   * @param transactions
   * @param expected_block_height - change expected height when using additional
   * initial blocks
   */
  void sendTransactions(std::vector<iroha::model::Transaction> transactions) {
    // generate expected proposal
    expected_proposal = std::make_shared<iroha::model::Proposal>(transactions);
    expected_proposal->height = 2;

    // generate expected block
    expected_block = iroha::model::Block{};
    expected_block.height = expected_proposal->height;
    expected_block.prev_hash = genesis_block.hash;
    expected_block.transactions = transactions;
    expected_block.txs_number = transactions.size();
    expected_block.created_ts = 0;
    expected_block.merkle_root.fill(0);
    expected_block.hash = iroha::hash(expected_block);
    irohad->getCryptoProvider()->sign(expected_block);

    // send transactions to torii
    for (const auto &tx : transactions) {
      auto pb_tx =
          iroha::model::converters::PbTransactionFactory().serialize(tx);

      google::protobuf::Empty response;
      irohad->getCommandService()->ToriiAsync(pb_tx, response);
    }
  }

  void validate() {
    // verify proposal
    auto proposal_wrapper = make_test_subscriber<CallExact>(
        irohad->getPeerCommunicationService()->on_proposal(), 1);
    proposal_wrapper.subscribe(
        [this](auto proposal) { proposals.push_back(proposal); });

    // verify commit and block
    auto commit_wrapper = make_test_subscriber<CallExact>(
        irohad->getPeerCommunicationService()->on_commit(), 1);
    commit_wrapper.subscribe([this](auto commit) {
      auto block_wrapper = make_test_subscriber<CallExact>(commit, 1);
      block_wrapper.subscribe([this](auto block) { blocks.push_back(block); });
    });
    irohad->getPeerCommunicationService()->on_commit().subscribe(
        [this](auto) { cv.notify_one(); });

    // wait for commit
    std::unique_lock<std::mutex> lk(m);

    cv.wait_for(lk, 10s, [this] { return blocks.size() == 1; });

    ASSERT_TRUE(proposal_wrapper.validate());
    ASSERT_EQ(1, proposals.size());
    ASSERT_EQ(*expected_proposal, proposals.front());

    ASSERT_TRUE(commit_wrapper.validate());
    ASSERT_EQ(1, blocks.size());
    ASSERT_EQ(expected_block, blocks.front());
  }

  iroha::keypair_t createNewAccountKeypair(
      const std::string &accountName) const {
    auto manager = iroha::KeysManagerImpl(accountName);
    EXPECT_TRUE(manager.createKeys(accountName));
    EXPECT_TRUE(manager.loadKeys().has_value());
    return *manager.loadKeys();
  }

  std::shared_ptr<TestIrohad> irohad;

  std::condition_variable cv;
  std::mutex m;

  std::shared_ptr<iroha::model::Proposal> expected_proposal;
  iroha::model::Block genesis_block, expected_block;

  std::vector<iroha::model::Proposal> proposals;
  std::vector<iroha::model::Block> blocks;

  std::shared_ptr<iroha::KeysManager> manager;
};

#endif
