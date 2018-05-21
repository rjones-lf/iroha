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

#include "framework/integration_framework/integration_test_framework.hpp"

#include <memory>

#include "backend/protobuf/block.hpp"
#include "backend/protobuf/queries/proto_query.hpp"
#include "backend/protobuf/query_responses/proto_query_response.hpp"
#include "backend/protobuf/transaction.hpp"
#include "backend/protobuf/transaction_responses/proto_tx_response.hpp"
#include "builders/protobuf/block.hpp"
#include "builders/protobuf/proposal.hpp"
#include "builders/protobuf/transaction.hpp"
#include "common/files.hpp"
#include "cryptography/crypto_provider/crypto_defaults.hpp"
#include "cryptography/default_hash_provider.hpp"
#include "datetime/time.hpp"
#include "framework/integration_framework/iroha_instance.hpp"
#include "framework/integration_framework/test_irohad.hpp"
// TODO (@l4l) IR-874 create more comfort way for permission-dependent proto
// building
#include "validators/permissions.hpp"

using namespace shared_model::crypto;
using namespace std::literals::string_literals;

namespace integration_framework {

  const std::string IntegrationTestFramework::kDefaultDomain = "test";
  const std::string IntegrationTestFramework::kDefaultRole = "user";
  const std::string IntegrationTestFramework::kAdminName = "admin";
  const std::string IntegrationTestFramework::kAdminId = "admin@test";
  const std::string IntegrationTestFramework::kAssetName = "coin";

  IntegrationTestFramework::IntegrationTestFramework(
      size_t maximum_proposal_size,
      std::function<void(integration_framework::IntegrationTestFramework &)>
          deleter,
      bool mst_support,
      const std::string &block_store_path)
      : iroha_instance_(std::make_shared<IrohaInstance>(mst_support, block_store_path)),
        maximum_proposal_size_(maximum_proposal_size),
        deleter_(deleter) {}

  IntegrationTestFramework::~IntegrationTestFramework() {
    if (deleter_) {
      deleter_(*this);
    }
    // the code below should be executed anyway in order to prevent app hang
    if (iroha_instance_ and iroha_instance_->instance_) {
      iroha_instance_->instance_->terminate();
    }
  }

  shared_model::proto::Block IntegrationTestFramework::defaultBlock(
      const shared_model::crypto::Keypair &key) {
    auto genesis_tx =
        shared_model::proto::TransactionBuilder()
            .creatorAccountId(kAdminId)
            .createdTime(iroha::time::now())
            .addPeer("0.0.0.0:50541", key.publicKey())
            .createRole(kDefaultRole,
                        // TODO (@l4l) IR-874 create more comfort way for
                        // permission-dependent proto building
                        std::vector<std::string>{
                            shared_model::permissions::role_perm_group.begin(),
                            shared_model::permissions::role_perm_group.end()})
            .createDomain(kDefaultDomain, kDefaultRole)
            .createAccount(kAdminName, kDefaultDomain, key.publicKey())
            .createAsset(kAssetName, kDefaultDomain, 1)
            .quorum(1)
            .build()
            .signAndAddSignature(key);
    auto genesis_block =
        shared_model::proto::BlockBuilder()
            .transactions(
                std::vector<shared_model::proto::Transaction>{genesis_tx})
            .height(1)
            .prevHash(DefaultHashProvider::makeHash(Blob("")))
            .createdTime(iroha::time::now())
            .build()
            .signAndAddSignature(key);
    return genesis_block;
  }

  IntegrationTestFramework &IntegrationTestFramework::setInitialState(
      const Keypair &keypair) {
    return setInitialState(keypair,
                           IntegrationTestFramework::defaultBlock(keypair));
  }

  IntegrationTestFramework &IntegrationTestFramework::setInitialState(
      const Keypair &keypair, const shared_model::interface::Block &block) {
    initPipeline(keypair);
    iroha_instance_->makeGenesis(block);
    log_->info("added genesis block");
    subscribeQueuesAndRun();
    return *this;
  }

  IntegrationTestFramework &IntegrationTestFramework::recoverState(
      const Keypair &keypair) {
    initPipeline(keypair);
    iroha_instance_->instance_->init();
    subscribeQueuesAndRun();
    return *this;
  }

  void IntegrationTestFramework::initPipeline(
      const shared_model::crypto::Keypair &keypair) {
    log_->info("init state");
    // peer initialization
    iroha_instance_->initPipeline(keypair, maximum_proposal_size_);
    log_->info("created pipeline");
    iroha_instance_->instance_->resetOrderingService();
  }

  void IntegrationTestFramework::subscribeQueuesAndRun() {
    // subscribing for components

    iroha_instance_->getIrohaInstance()
        ->getPeerCommunicationService()
        ->on_proposal()
        .subscribe([this](auto proposal) {
          proposal_queue_.push(proposal);
          log_->info("proposal");
          queue_cond.notify_all();
        });

    iroha_instance_->getIrohaInstance()
        ->getPeerCommunicationService()
        ->on_commit()
        .subscribe([this](auto commit_observable) {
          commit_observable.subscribe([this](auto committed_block) {
            block_queue_.push(committed_block);
            log_->info("block");
            queue_cond.notify_all();
          });
          log_->info("commit");
          queue_cond.notify_all();
        });

    // start instance
    iroha_instance_->run();
    log_->info("run iroha");
  }

  shared_model::proto::TransactionResponse
  IntegrationTestFramework::getTxStatus(
      const shared_model::crypto::Hash &hash) {
    iroha::protocol::TxStatusRequest request;
    request.set_tx_hash(shared_model::crypto::toBinaryString(hash));
    iroha::protocol::ToriiResponse response;
    iroha_instance_->getIrohaInstance()->getCommandService()->Status(request,
                                                                     response);
    return shared_model::proto::TransactionResponse(std::move(response));
  }

  IntegrationTestFramework &IntegrationTestFramework::sendTx(
      const shared_model::proto::Transaction &tx,
      std::function<void(const shared_model::proto::TransactionResponse &)>
          validation) {
    log_->info("send transaction");
    iroha_instance_->getIrohaInstance()->getCommandService()->Torii(
        tx.getTransport());
    // fetch status of transaction
    shared_model::proto::TransactionResponse status = getTxStatus(tx.hash());

    // check validation function
    validation(status);
    return *this;
  }

  IntegrationTestFramework &IntegrationTestFramework::sendTx(
      const shared_model::proto::Transaction &tx) {
    sendTx(tx, [](const auto &) {});
    return *this;
  }

  IntegrationTestFramework &IntegrationTestFramework::sendQuery(
      const shared_model::proto::Query &qry,
      std::function<void(const shared_model::proto::QueryResponse &)>
          validation) {
    log_->info("send query");

    iroha::protocol::QueryResponse response;
    iroha_instance_->getIrohaInstance()->getQueryService()->Find(
        qry.getTransport(), response);
    auto query_response =
        shared_model::proto::QueryResponse(std::move(response));

    validation(query_response);
    return *this;
  }

  IntegrationTestFramework &IntegrationTestFramework::sendQuery(
      const shared_model::proto::Query &qry) {
    sendQuery(qry, [](const auto &) {});
    return *this;
  }

  IntegrationTestFramework &IntegrationTestFramework::checkProposal(
      std::function<void(const ProposalType &)> validation) {
    log_->info("check proposal");
    // fetch first proposal from proposal queue
    ProposalType proposal;
    fetchFromQueue(
        proposal_queue_, proposal, proposal_waiting, "missed proposal");
    validation(proposal);
    return *this;
  }

  IntegrationTestFramework &IntegrationTestFramework::skipProposal() {
    checkProposal([](const auto &) {});
    return *this;
  }

  IntegrationTestFramework &IntegrationTestFramework::checkBlock(
      std::function<void(const BlockType &)> validation) {
    // fetch first from block queue
    log_->info("check block");
    BlockType block;
    fetchFromQueue(block_queue_, block, block_waiting, "missed block");
    validation(block);
    return *this;
  }

  IntegrationTestFramework &IntegrationTestFramework::skipBlock() {
    checkBlock([](const auto &) {});
    return *this;
  }

  void IntegrationTestFramework::done() {
    log_->info("done");
    if (iroha_instance_->instance_ and iroha_instance_->instance_->storage) {
      iroha_instance_->instance_->storage->dropStorage();
    }
  }
}  // namespace integration_framework
