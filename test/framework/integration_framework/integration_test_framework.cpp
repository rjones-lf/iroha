/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "framework/integration_framework/integration_test_framework.hpp"

#include <memory>

#include <boost/thread/barrier.hpp>

#include "backend/protobuf/block.hpp"
#include "backend/protobuf/queries/proto_query.hpp"
#include "backend/protobuf/query_responses/proto_query_response.hpp"
#include "backend/protobuf/transaction.hpp"
#include "backend/protobuf/transaction_responses/proto_tx_response.hpp"
#include "builders/protobuf/transaction.hpp"
#include "builders/protobuf/transaction_sequence_builder.hpp"
#include "common/files.hpp"
#include "cryptography/crypto_provider/crypto_defaults.hpp"
#include "cryptography/default_hash_provider.hpp"
#include "datetime/time.hpp"
#include "framework/integration_framework/iroha_instance.hpp"
#include "framework/integration_framework/test_irohad.hpp"
#include "framework/result_fixture.hpp"
#include "interfaces/permissions.hpp"
#include "module/shared_model/builders/protobuf/block.hpp"
#include "module/shared_model/builders/protobuf/proposal.hpp"
#include "synchronizer/synchronizer_common.hpp"

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
      const boost::optional<std::string> &dbname,
      std::function<void(integration_framework::IntegrationTestFramework &)>
          deleter,
      bool mst_support,
      const std::string &block_store_path,
      milliseconds proposal_waiting,
      milliseconds block_waiting)
      : iroha_instance_(std::make_shared<IrohaInstance>(
            mst_support, block_store_path, dbname)),
        proposal_waiting(proposal_waiting),
        block_waiting(block_waiting),
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
    shared_model::interface::RolePermissionSet all_perms{};
    for (size_t i = 0; i < all_perms.size(); ++i) {
      auto perm = static_cast<shared_model::interface::permissions::Role>(i);
      all_perms.set(perm);
    }
    auto genesis_tx =
        shared_model::proto::TransactionBuilder()
            .creatorAccountId(kAdminId)
            .createdTime(iroha::time::now())
            .addPeer("0.0.0.0:50541", key.publicKey())
            .createRole(kDefaultRole, all_perms)
            .createDomain(kDefaultDomain, kDefaultRole)
            .createAccount(kAdminName, kDefaultDomain, key.publicKey())
            .createAsset(kAssetName, kDefaultDomain, 1)
            .quorum(1)
            .build()
            .signAndAddSignature(key)
            .finish();
    auto genesis_block =
        shared_model::proto::BlockBuilder()
            .transactions(
                std::vector<shared_model::proto::Transaction>{genesis_tx})
            .height(1)
            .prevHash(DefaultHashProvider::makeHash(Blob("")))
            .createdTime(iroha::time::now())
            .build()
            .signAndAddSignature(key)
            .finish();
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
        ->on_verified_proposal()
        .subscribe([this](auto verified_proposal_and_errors) {
          verified_proposal_queue_.push(verified_proposal_and_errors->first);
          log_->info("verified proposal");
          queue_cond.notify_all();
        });

    iroha_instance_->getIrohaInstance()
        ->getPeerCommunicationService()
        ->on_commit()
        .subscribe([this](auto commit_event) {
          commit_event.synced_blocks.subscribe([this](auto committed_block) {
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

  IntegrationTestFramework &IntegrationTestFramework::getTxStatus(
      const shared_model::crypto::Hash &hash,
      std::function<void(const shared_model::proto::TransactionResponse &)>
          validation) {
    iroha::protocol::TxStatusRequest request;
    request.set_tx_hash(shared_model::crypto::toBinaryString(hash));
    iroha::protocol::ToriiResponse response;
    iroha_instance_->getIrohaInstance()->getCommandServiceTransport()->Status(
        nullptr, &request, &response);
    validation(shared_model::proto::TransactionResponse(std::move(response)));
    return *this;
  }

  IntegrationTestFramework &IntegrationTestFramework::sendTx(
      const shared_model::proto::Transaction &tx,
      std::function<void(const shared_model::proto::TransactionResponse &)>
          validation) {
    log_->info("send transaction");

    // Required for StatusBus synchronization
    boost::barrier bar1(2);
    auto bar2 = std::make_shared<boost::barrier>(2);
    iroha_instance_->instance_->getStatusBus()
        ->statuses()
        .filter([&](auto s) { return s->transactionHash() == tx.hash(); })
        .take(1)
        .subscribe([&bar1, b2 = std::weak_ptr<boost::barrier>(bar2)](auto s) {
          bar1.wait();
          if (auto lock = b2.lock()) {
            lock->wait();
          }
        });

    iroha_instance_->getIrohaInstance()->getCommandServiceTransport()->Torii(
        nullptr, &tx.getTransport(), nullptr);
    // make sure that the first (stateless) status is come
    bar1.wait();
    // fetch status of transaction
    getTxStatus(tx.hash(), [&validation, &bar2](auto &status) {
      // make sure that the following statuses (stateful/committed)
      // isn't reached the bus yet
      bar2->wait();

      // check validation function
      validation(status);
    });
    return *this;
  }

  IntegrationTestFramework &IntegrationTestFramework::sendTx(
      const shared_model::proto::Transaction &tx) {
    sendTx(tx, [](const auto &) {});
    return *this;
  }

  IntegrationTestFramework &IntegrationTestFramework::sendTxAwait(
      const shared_model::proto::Transaction &tx,
      std::function<void(const BlockType &)> check) {
    sendTx(tx).skipProposal().skipVerifiedProposal().checkBlock(check);
    return *this;
  }

  IntegrationTestFramework &IntegrationTestFramework::sendTxSequence(
      const shared_model::interface::TransactionSequence &tx_sequence,
      std::function<void(std::vector<shared_model::proto::TransactionResponse>
                             &)> validation) {
    log_->info("send transactions");
    const auto &transactions = tx_sequence.transactions();

    std::mutex m;
    std::condition_variable cv;
    bool processed = false;

    // subscribe on status bus and save all stateless statuses into a vector
    std::vector<shared_model::proto::TransactionResponse> statuses;
    iroha_instance_->instance_->getStatusBus()
        ->statuses()
        .filter([&transactions](auto s) {
          // filter statuses for transactions from sequence
          auto it = std::find_if(
              transactions.begin(), transactions.end(), [&s](const auto tx) {
                // check if status is either stateless valid or failed
                bool is_stateless_status = iroha::visit_in_place(
                    s->get(),
                    [](const shared_model::interface::StatelessFailedTxResponse
                           &stateless_failed_response) { return true; },
                    [](const shared_model::interface::StatelessValidTxResponse
                           &stateless_valid_response) { return true; },
                    [](const auto &other_responses) { return false; });
                return is_stateless_status
                    and s->transactionHash() == tx->hash();
              });
          return it != transactions.end();
        })
        .take(transactions.size())
        .subscribe(
            [&statuses](auto s) {
              statuses.push_back(*std::static_pointer_cast<
                                 shared_model::proto::TransactionResponse>(s));
            },
            [&cv, &m, &processed] {
              std::lock_guard<std::mutex> lock(m);
              processed = true;
              cv.notify_all();
            });

    // put all transactions to the TxList and send them to iroha
    iroha::protocol::TxList tx_list;
    for (const auto &tx : transactions) {
      auto proto_tx =
          std::static_pointer_cast<shared_model::proto::Transaction>(tx)
              ->getTransport();
      *tx_list.add_transactions() = proto_tx;
    }
    iroha_instance_->getIrohaInstance()
        ->getCommandServiceTransport()
        ->ListTorii(nullptr, &tx_list, nullptr);

    std::unique_lock<std::mutex> lk(m);
    cv.wait(lk, [&] { return processed; });

    validation(statuses);
    return *this;
  }

  IntegrationTestFramework &IntegrationTestFramework::sendTxSequenceAwait(
      const shared_model::interface::TransactionSequence &tx_sequence,
      std::function<void(const BlockType &)> check) {
    sendTxSequence(tx_sequence)
        .skipProposal()
        .skipVerifiedProposal()
        .checkBlock(check);
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

  IntegrationTestFramework &IntegrationTestFramework::checkVerifiedProposal(
      std::function<void(const ProposalType &)> validation) {
    log_->info("check verified proposal");
    // fetch first proposal from proposal queue
    ProposalType verified_proposal;
    fetchFromQueue(verified_proposal_queue_,
                   verified_proposal,
                   proposal_waiting,
                   "missed verified proposal");
    validation(verified_proposal);
    return *this;
  }

  IntegrationTestFramework &IntegrationTestFramework::skipVerifiedProposal() {
    checkVerifiedProposal([](const auto &) {});
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
      boost::filesystem::remove_all(iroha_instance_->block_store_dir_);
    }
  }
}  // namespace integration_framework
