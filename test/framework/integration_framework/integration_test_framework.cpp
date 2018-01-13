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

#include "framework/integration_framework/integration_test_framework.hpp"
#include "framework/integration_framework/iroha_instance.hpp"

namespace integration_framework {
  IntegrationTestFramework &IntegrationTestFramework::setInitialState(
      const iroha::keypair_t &keypair) {
    iroha::model::Block b;
    iroha::model::generators::CommandGenerator gen;
    iroha::model::Transaction tx;
    for (auto v :
         {gen.generateAddPeer("0.0.0.0:0", keypair.pubkey),
          gen.generateCreateUserRole(default_role),
          gen.generateCreateDomain(default_domain, default_role),
          gen.generateCreateAccount("admin", default_domain, keypair.pubkey),
          gen.generateCreateAsset("coin", default_domain, 1)}) {
      tx.commands.push_back(std::move(v));
    }
    b.transactions = {tx};
    b.height = 1;
    b.txs_number = 1;
    b.hash = iroha::hash(b);
    return setInitialState(keypair, b);
  }

  IntegrationTestFramework &IntegrationTestFramework::setInitialState(
      const iroha::keypair_t &keypair, const iroha::model::Block &block) {
    log_->info("init state");
    // peer initialization
    iroha_instance_->initPipeline(keypair);
    log_->info("created pipeline");
    // iroha_instance_->clearLedger();
    // log_->info("cleared ledger");
    iroha_instance_->makeGenesis(block);
    log_->info("added genesis block");

    // subscribing for components

    iroha_instance_->getIrohaInstance()
        ->getPeerCommunicationService()
        ->on_proposal()
        .subscribe([this](auto proposal) {
          proposal_queue_.push(
              std::make_shared<iroha::model::Proposal>(proposal));
        });

    iroha_instance_->getIrohaInstance()
        ->getPeerCommunicationService()
        ->on_commit()
        .subscribe([this](auto commit_observable) {
          commit_observable.subscribe([this](auto committed_block) {
            block_queue_.push(
                std::make_shared<iroha::model::Block>(committed_block));
          });
        });

    // start instance
    iroha_instance_->run();
    log_->info("run iroha");
    return *this;
  }

  IntegrationTestFramework &IntegrationTestFramework::addUser(
      std::string account_id,
      const iroha::keypair_t &keypair,
      std::vector<iroha::pubkey_t> signatories) {
    iroha::model::generators::CommandGenerator gen;
    iroha::model::Transaction tx;

    // add an account
    tx.commands.push_back(
        gen.generateCreateAccount(account_id, default_domain, keypair.pubkey));

    // set the account quorum
    tx.commands.push_back(
        gen.generateSetQuorum(account_id, signatories.size() + 1));

    // add the signatories to the account
    std::for_each(signatories.begin(), signatories.end(), [&](auto &s) {
      tx.commands.push_back(gen.generateAddSignatory(account_id, s));
    });

    tx.created_ts = iroha::time::now();
    iroha::model::ModelCryptoProviderImpl(keypair).sign(tx);
    return sendTx(tx);
  }

  IntegrationTestFramework &IntegrationTestFramework::sendTx(
      iroha::model::Transaction tx) {
    sendTx(tx, [](auto) {});
    return *this;
  }

  IntegrationTestFramework &IntegrationTestFramework::sendQuery(
      const iroha::model::Query &qry) {
    sendQuery(qry, [](const auto &) {});
    return *this;
  }

  IntegrationTestFramework &IntegrationTestFramework::skipProposal() {
    checkProposal([](const iroha::model::Proposal &) {});
    return *this;
  }

  IntegrationTestFramework &IntegrationTestFramework::skipBlock() {
    checkBlock([](const iroha::model::Block &) {});
    return *this;
  }

  void IntegrationTestFramework::done() {
    log_->info("done");
    iroha_instance_->instance_->storage->dropStorage();
  }
}  // namespace integration_framework
