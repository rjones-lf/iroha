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
#include "builders/protobuf/block.hpp"
#include "builders/protobuf/proposal.hpp"
#include "builders/protobuf/transaction.hpp"
#include "cryptography/hash_providers/sha3_256.hpp"
#include "datetime/time.hpp"
// TODO (@l4l) IR-874 create more confort way for permssion-dependent proto
// building
#include "model/permissions.hpp"

using namespace shared_model::crypto;
using namespace std::literals::string_literals;

namespace integration_framework {
  IntegrationTestFramework &IntegrationTestFramework::setInitialState(
      const shared_model::crypto::Keypair &key) {
    auto genesis_tx =
        shared_model::proto::TransactionBuilder()
            .creatorAccountId("admin@test")
            .txCounter(1)
            .createdTime(iroha::time::now())
            .addPeer("0.0.0.0:10001", key.publicKey())
            .createRole(
                default_role,
                // TODO (@l4l) IR-874 create more confort way for
                // permssion-dependent proto building
                std::vector<std::string>{iroha::model::can_create_domain,
                                         iroha::model::can_create_account,
                                         iroha::model::can_add_asset_qty,
                                         iroha::model::can_add_peer,
                                         iroha::model::can_receive,
                                         iroha::model::can_transfer})
            .createDomain(default_domain, default_role)
            .createAccount("admin", default_domain, key.publicKey())
            .createAsset("coin", default_domain, 1)
            .build()
            .signAndAddSignature(key);
    auto genesis_block =
        shared_model::proto::BlockBuilder()
            .transactions(
                std::vector<shared_model::proto::Transaction>{genesis_tx})
            .txNumber(1)
            .height(1)
            .prevHash(Sha3_256::makeHash(Blob("")))
            .createdTime(iroha::time::now())
            .build()
            .signAndAddSignature(key);
    return setInitialState(key, genesis_block);
  }

  IntegrationTestFramework &IntegrationTestFramework::setInitialState(
      const Keypair &keypair, const shared_model::interface::Block &block) {
    log_->info("init state");
    // peer initialization
    std::shared_ptr<iroha::keypair_t> old_key(keypair.makeOldModel());
    iroha_instance_->initPipeline(*old_key, maximum_block_size_);
    log_->info("created pipeline");
    // iroha_instance_->clearLedger();
    // log_->info("cleared ledger");
    iroha_instance_->instance_->resetOrderingService();
    std::shared_ptr<iroha::model::Block> old_block(block.makeOldModel());
    iroha_instance_->makeGenesis(*old_block);
    log_->info("added genesis block");

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
    return *this;
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
      const shared_model::proto::Transaction &tx) {
    sendTx(tx, [](const auto &) {});
    return *this;
  }

  IntegrationTestFramework &IntegrationTestFramework::sendQuery(
      const shared_model::proto::Query &qry) {
    sendQuery(qry, [](const auto &) {});
    return *this;
  }

  IntegrationTestFramework &IntegrationTestFramework::skipProposal() {
    checkProposal([](const auto &) {});
    return *this;
  }

  IntegrationTestFramework &IntegrationTestFramework::skipBlock() {
    checkBlock([](const auto &) {});
    return *this;
  }

  void IntegrationTestFramework::done() {
    log_->info("done");
    iroha_instance_->instance_->storage->dropStorage();
  }
}  // namespace integration_framework
