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

#ifndef TX_PIPELINE_INTEGRATION_TEST_FIXTURE_HPP
#define TX_PIPELINE_INTEGRATION_TEST_FIXTURE_HPP

#include "crypto/keys_manager_impl.hpp"
#include "cryptography/ed25519_sha3_impl/internal/sha3_hash.hpp"
#include "datetime/time.hpp"
#include "framework/test_subscriber.hpp"
#include "integration/pipeline/test_irohad.hpp"
#include "main/application.hpp"
#include "main/raw_block_loader.hpp"
#include "model/generators/block_generator.hpp"
#include "module/irohad/ametsuchi/ametsuchi_fixture.hpp"

class TxPipelineIntegrationTestFixture
    : public iroha::ametsuchi::AmetsuchiTest {
 public:
  TxPipelineIntegrationTestFixture();

  /**
   * @param transactions in order
   */
  void sendTxsInOrderAndValidate(
      const std::vector<iroha::model::Transaction> &transactions);

  iroha::keypair_t createNewAccountKeypair(
      const std::string &accountName) const;

  std::shared_ptr<TestIrohad> irohad;

  std::condition_variable cv;
  std::mutex m;

  std::vector<std::shared_ptr<shared_model::interface::Proposal>> proposals;
  std::vector<std::shared_ptr<shared_model::interface::Block>> blocks;

  std::unique_ptr<framework::test_subscriber::TestSubscriber<
      std::shared_ptr<shared_model::interface::Proposal>>>
      proposal_wrapper;
  std::unique_ptr<framework::test_subscriber::TestSubscriber<iroha::Commit>>
      commit_wrapper;

  iroha::model::Block genesis_block;
  std::vector<iroha::model::Proposal> expected_proposals;
  std::vector<iroha::model::Block> expected_blocks;

  std::shared_ptr<iroha::KeysManager> manager;

  std::atomic_bool duplicate_sent{false};
  size_t next_height_count = 2;

  const size_t default_port = 50541;

 private:
  void setTestSubscribers(size_t num_blocks);

  void sendTransaction(const iroha::model::Transaction &transaction);
};

#endif
