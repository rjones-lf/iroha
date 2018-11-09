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
#include "framework/common_constants.hpp"
#include "framework/integration_framework/iroha_instance.hpp"
#include "framework/integration_framework/test_irohad.hpp"
#include "framework/result_fixture.hpp"
#include "interfaces/permissions.hpp"
#include "module/shared_model/builders/protobuf/block.hpp"
#include "module/shared_model/builders/protobuf/proposal.hpp"
#include "synchronizer/synchronizer_common.hpp"

using namespace shared_model::crypto;
using namespace std::literals::string_literals;
using namespace common_constants;

static size_t kToriiPort = 11501;

namespace integration_framework {

  IntegrationTestFramework::IntegrationTestFramework(
      size_t maximum_proposal_size,
      const boost::optional<std::string> &dbname,
      std::function<void(integration_framework::IntegrationTestFramework &)>
          deleter,
      bool mst_support,
      const std::string &block_store_path,
      milliseconds proposal_waiting,
      milliseconds block_waiting,
      milliseconds tx_response_waiting)
      : iroha_instance_(std::make_shared<IrohaInstance>(
            mst_support, block_store_path, kToriiPort, dbname)),
        command_client_("127.0.0.1", kToriiPort),
        query_client_("127.0.0.1", kToriiPort),
        proposal_waiting(proposal_waiting),
        block_waiting(block_waiting),
        tx_response_waiting(tx_response_waiting),
        maximum_proposal_size_(maximum_proposal_size),
        deleter_(deleter) {}

  IntegrationTestFramework::~IntegrationTestFramework() {
    if (deleter_) {
      deleter_(*this);
    }
    // the code below should be executed anyway in order to prevent app hang
    if (iroha_instance_ and iroha_instance_->getIrohaInstance()) {
      iroha_instance_->getIrohaInstance()->terminate(
          std::chrono::system_clock::now());
    }
  }

  shared_model::proto::Block IntegrationTestFramework::defaultBlock(
      const shared_model::crypto::Keypair &key) {
    shared_model::interface::RolePermissionSet all_perms{};
    for (size_t i = 0; i < all_perms.size(); ++i) {
      auto perm = static_cast<shared_model::interface::permissions::Role>(i);
      all_perms.set(perm);
    }
    auto genesis_tx = shared_model::proto::TransactionBuilder()
                          .creatorAccountId(kAdminId)
                          .createdTime(iroha::time::now())
                          .addPeer("0.0.0.0:50541", key.publicKey())
                          .createRole(kAdminRole, all_perms)
                          .createRole(kDefaultRole, {})
                          .createDomain(kDomain, kDefaultRole)
                          .createAccount(kAdminName, kDomain, key.publicKey())
                          .detachRole(kAdminId, kDefaultRole)
                          .appendRole(kAdminId, kAdminRole)
                          .createAsset(kAssetName, kDomain, 1)
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
    iroha_instance_->getIrohaInstance()->init();
    subscribeQueuesAndRun();
    return *this;
  }

  void IntegrationTestFramework::initPipeline(
      const shared_model::crypto::Keypair &keypair) {
    log_->info("init state");
    // peer initialization
    iroha_instance_->initPipeline(keypair, maximum_proposal_size_);
    log_->info("created pipeline");
  }

  void IntegrationTestFramework::subscribeQueuesAndRun() {
    // subscribing for components

    auto proposals = iroha_instance_->getIrohaInstance()
                         ->getPeerCommunicationService()
                         ->on_proposal();

    proposals
        .filter([](auto proposal) {
          return boost::size(proposal->transactions()) != 0;
        })
        .subscribe([this](auto proposal) {
          proposal_queue_.push(proposal);
          log_->info("proposal");
          queue_cond.notify_all();
        });

    auto proposal_flat_map =
        [](auto t) -> rxcpp::observable<std::tuple_element_t<0, decltype(t)>> {
      if (boost::size(std::get<1>(t)->transactions()) != 0) {
        return rxcpp::observable<>::just(std::get<0>(t));
      }
      return rxcpp::observable<>::empty<std::tuple_element_t<0, decltype(t)>>();
    };

    iroha_instance_->getIrohaInstance()
        ->getPeerCommunicationService()
        ->on_verified_proposal()
        .zip(proposals)
        .flat_map(proposal_flat_map)
        .subscribe([this](auto verified_proposal_and_errors) {
          verified_proposal_queue_.push(verified_proposal_and_errors->first);
          log_->info("verified proposal");
          queue_cond.notify_all();
        });

    iroha_instance_->getIrohaInstance()
        ->getPeerCommunicationService()
        ->on_commit()
        .zip(proposals)
        .flat_map(proposal_flat_map)
        .subscribe([this](auto commit_event) {
          commit_event.synced_blocks.subscribe([this](auto committed_block) {
            block_queue_.push(committed_block);
            log_->info("block");
            queue_cond.notify_all();
          });
          log_->info("commit");
          queue_cond.notify_all();
        });
    iroha_instance_->getIrohaInstance()->getStatusBus()->statuses().subscribe(
        [this](auto response) {
          responses_queues_[response->transactionHash().hex()].push(response);
          log_->info("response");
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
    command_client_.Status(request, response);
    validation(shared_model::proto::TransactionResponse(std::move(response)));
    return *this;
  }

  IntegrationTestFramework &IntegrationTestFramework::sendTx(
      const shared_model::proto::Transaction &tx,
      std::function<void(const shared_model::proto::TransactionResponse &)>
          validation) {
    log_->info("sending transaction");
    log_->debug(tx.toString());

    // Required for StatusBus synchronization
    boost::barrier bar1(2);
    auto bar2 = std::make_shared<boost::barrier>(2);
    iroha_instance_->getIrohaInstance()
        ->getStatusBus()
        ->statuses()
        .filter([&](auto s) { return s->transactionHash() == tx.hash(); })
        .take(1)
        .subscribe([&bar1, b2 = std::weak_ptr<boost::barrier>(bar2)](auto s) {
          bar1.wait();
          if (auto lock = b2.lock()) {
            lock->wait();
          }
        });

    command_client_.Torii(tx.getTransport());
    // make sure that the first (stateless) status has come
    bar1.wait();
    // fetch status of transaction
    getTxStatus(tx.hash(), [&validation, &bar2](auto &status) {
      // make sure that the following statuses (stateful/committed)
      // haven't reached the bus yet
      bar2->wait();

      // check validation function
      validation(status);
    });
    return *this;
  }

  IntegrationTestFramework &IntegrationTestFramework::sendTx(
      const shared_model::proto::Transaction &tx) {
    sendTx(tx, [this](const auto &status) {
            if (!status.errorMessage().empty()) {
                 log_->debug("Got error while sending transaction: "
                                + status.errorMessage());
            }
    });
    return *this;
  }

  IntegrationTestFramework &IntegrationTestFramework::sendTxAwait(
      const shared_model::proto::Transaction &tx) {
    return sendTxAwait(tx, [](const auto &) {});
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
    iroha_instance_->getIrohaInstance()
        ->getStatusBus()
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
    command_client_.ListTorii(tx_list);

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
    log_->debug(qry.toString());

    iroha::protocol::QueryResponse response;
    query_client_.Find(qry.getTransport(), response);
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

  IntegrationTestFramework &IntegrationTestFramework::checkStatus(
      const shared_model::interface::types::HashType &tx_hash,
      std::function<void(const shared_model::proto::TransactionResponse &)>
          validation) {
    // fetch first response associated with the tx from related queue
    TxResponseType response;
    fetchFromQueue(responses_queues_[tx_hash.hex()],
                   response,
                   tx_response_waiting,
                   "missed status");
    validation(static_cast<const shared_model::proto::TransactionResponse &>(
        *response));
    return *this;
  }

  void IntegrationTestFramework::done() {
    log_->info("done");
    if (iroha_instance_->getIrohaInstance()
        and iroha_instance_->getIrohaInstance()->storage) {
      iroha_instance_->getIrohaInstance()->storage->dropStorage();
      boost::filesystem::remove_all(iroha_instance_->block_store_dir_);
    }
  }
}  // namespace integration_framework
