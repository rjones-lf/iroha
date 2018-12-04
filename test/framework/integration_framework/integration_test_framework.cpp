/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "framework/integration_framework/integration_test_framework.hpp"

#include <limits>
#include <memory>

#include <boost/assert.hpp>
#include <boost/thread/barrier.hpp>
#include "backend/protobuf/block.hpp"
#include "backend/protobuf/common_objects/proto_common_objects_factory.hpp"
#include "backend/protobuf/proto_transport_factory.hpp"
#include "backend/protobuf/queries/proto_query.hpp"
#include "backend/protobuf/query_responses/proto_query_response.hpp"
#include "backend/protobuf/transaction.hpp"
#include "backend/protobuf/transaction_responses/proto_tx_response.hpp"
#include "builders/protobuf/transaction.hpp"
#include "builders/protobuf/transaction_sequence_builder.hpp"
#include "common/files.hpp"
#include "consensus/yac/transport/impl/network_impl.hpp"
#include "cryptography/crypto_provider/crypto_defaults.hpp"
#include "cryptography/default_hash_provider.hpp"
#include "datetime/time.hpp"
#include "framework/common_constants.hpp"
#include "framework/integration_framework/fake_peer/fake_peer.hpp"
#include "framework/integration_framework/iroha_instance.hpp"
#include "framework/integration_framework/port_guard.hpp"
#include "framework/integration_framework/test_irohad.hpp"
#include "framework/result_fixture.hpp"
#include "interfaces/iroha_internal/transaction_batch_factory_impl.hpp"
#include "interfaces/iroha_internal/transaction_batch_parser_impl.hpp"
#include "interfaces/permissions.hpp"
#include "module/irohad/ametsuchi/tx_presence_cache_stub.hpp"
#include "module/shared_model/builders/protobuf/block.hpp"
#include "module/shared_model/builders/protobuf/proposal.hpp"
#include "module/shared_model/validators/always_valid_validators.hpp"
#include "multi_sig_transactions/transport/mst_transport_grpc.hpp"
#include "network/impl/async_grpc_client.hpp"
#include "synchronizer/synchronizer_common.hpp"

using namespace shared_model::crypto;
using namespace std::literals::string_literals;
using namespace common_constants;

using AlwaysValidProtoCommonObjectsFactory =
    shared_model::proto::ProtoCommonObjectsFactory<
        shared_model::validation::AlwaysValidFieldValidator>;
using ProtoTransactionFactory = shared_model::proto::ProtoTransportFactory<
    shared_model::interface::Transaction,
    shared_model::proto::Transaction>;
using AbstractTransactionValidator =
    shared_model::validation::AbstractValidator<
        shared_model::interface::Transaction>;
using AlwaysValidInterfaceTransactionValidator =
    shared_model::validation::AlwaysValidModelValidator<
        shared_model::interface::Transaction>;
using AlwaysValidProtoTransactionValidator =
    shared_model::validation::AlwaysValidModelValidator<
        iroha::protocol::Transaction>;
using AlwaysMissingTxPresenceCache = iroha::ametsuchi::TxPresenceCacheStub<
    iroha::ametsuchi::tx_cache_status_responses::Missing>;

namespace {
  std::string kLocalHost = "127.0.0.1";
  constexpr size_t kDefaultToriiPort = 11501;
  constexpr size_t kDefaultInternalPort = 50541;
}  // namespace

namespace integration_framework {

  IntegrationTestFramework::IntegrationTestFramework(
      size_t maximum_proposal_size,
      const boost::optional<std::string> &dbname,
      bool cleanup_on_exit,
      bool mst_support,
      const std::string &block_store_path,
      milliseconds proposal_waiting,
      milliseconds block_waiting,
      milliseconds tx_response_waiting)
      : port_guard_(std::make_unique<PortGuard>()),
        torii_port_(port_guard_->getPort(kDefaultToriiPort)),
        internal_port_(port_guard_->getPort(kDefaultInternalPort)),
        iroha_instance_(std::make_shared<IrohaInstance>(mst_support,
                                                        block_store_path,
                                                        kLocalHost,
                                                        torii_port_,
                                                        internal_port_,
                                                        dbname)),
        command_client_(kLocalHost, torii_port_),
        query_client_(kLocalHost, torii_port_),
        async_call_(std::make_shared<AsyncCall>()),
        proposal_waiting(proposal_waiting),
        block_waiting(block_waiting),
        tx_response_waiting(tx_response_waiting),
        maximum_proposal_size_(maximum_proposal_size),
        common_objects_factory_(
            std::make_shared<AlwaysValidProtoCommonObjectsFactory>()),
        transaction_factory_(std::make_shared<ProtoTransactionFactory>(
            std::make_unique<AlwaysValidInterfaceTransactionValidator>(),
            std::make_unique<AlwaysValidProtoTransactionValidator>())),
        batch_parser_(std::make_shared<
                      shared_model::interface::TransactionBatchParserImpl>()),
        transaction_batch_factory_(
            std::make_shared<
                shared_model::interface::TransactionBatchFactoryImpl>()),
        tx_presence_cache_(std::make_shared<AlwaysMissingTxPresenceCache>()),
        yac_transport_(
            std::make_shared<iroha::consensus::yac::NetworkImpl>(async_call_)),
        cleanup_on_exit_(cleanup_on_exit) {}

  IntegrationTestFramework::~IntegrationTestFramework() {
    if (cleanup_on_exit_) {
      cleanup();
    }
    // the code below should be executed anyway in order to prevent app hang
    if (iroha_instance_ and iroha_instance_->getIrohaInstance()) {
      iroha_instance_->getIrohaInstance()->terminate();
    }
  }

  std::future<std::shared_ptr<FakePeer>>
  IntegrationTestFramework::addInitailPeer(
      const boost::optional<Keypair> &key) {
    fake_peers_promises_.emplace_back(std::promise<std::shared_ptr<FakePeer>>(),
                                      key);
    return fake_peers_promises_.back().first.get_future();
  }

  void IntegrationTestFramework::makeFakePeers() {
    if (fake_peers_promises_.size() == 0) {
      return;
    }
    log_->info("creating fake iroha peers");
    assert(this_peer_ && "this_peer_ is needed for fake peers initialization, "
        "but not set");
    for (auto &promise_and_key : fake_peers_promises_) {
      auto fake_peer =
          std::make_shared<FakePeer>(kLocalHost,
                                     port_guard_->getPort(kDefaultInternalPort),
                                     promise_and_key.second,
                                     this_peer_,
                                     common_objects_factory_,
                                     transaction_factory_,
                                     batch_parser_,
                                     transaction_batch_factory_,
                                     tx_presence_cache_);
      fake_peers_.emplace_back(fake_peer);
      promise_and_key.first.set_value(fake_peer);
    }
  }

  shared_model::proto::Block IntegrationTestFramework::defaultBlock(
      const shared_model::crypto::Keypair &key) const {
    shared_model::interface::RolePermissionSet all_perms{};
    for (size_t i = 0; i < all_perms.size(); ++i) {
      auto perm = static_cast<shared_model::interface::permissions::Role>(i);
      all_perms.set(perm);
    }
    auto genesis_tx_builder =
        shared_model::proto::TransactionBuilder()
            .creatorAccountId(kAdminId)
            .createdTime(iroha::time::now())
            .addPeer(kLocalHost + ":" + std::to_string(internal_port_),
                     key.publicKey())
            .createRole(kAdminRole, all_perms)
            .createRole(kDefaultRole, {})
            .createDomain(kDomain, kDefaultRole)
            .createAccount(kAdminName, kDomain, key.publicKey())
            .detachRole(kAdminId, kDefaultRole)
            .appendRole(kAdminId, kAdminRole)
            .createAsset(kAssetName, kDomain, 1)
            .quorum(1);
    // add fake peers
    for (const auto &fake_peer : fake_peers_) {
      genesis_tx_builder = genesis_tx_builder.addPeer(
          fake_peer->getAddress(), fake_peer->getKeypair().publicKey());
    };
    auto genesis_tx =
        genesis_tx_builder.build().signAndAddSignature(key).finish();
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
    initPipeline(keypair);
    iroha_instance_->makeGenesis(
        IntegrationTestFramework::defaultBlock(keypair));
    log_->info("added genesis block");
    subscribeQueuesAndRun();
    return *this;
  }

  IntegrationTestFramework &IntegrationTestFramework::setMstGossipParams(
      std::chrono::milliseconds mst_gossip_emitting_period,
      uint32_t mst_gossip_amount_per_once) {
    iroha_instance_->setMstGossipParams(mst_gossip_emitting_period,
                                        mst_gossip_amount_per_once);
    return *this;
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
    common_objects_factory_
        ->createPeer(kLocalHost + ":" + std::to_string(internal_port_),
                     keypair.publicKey())
        .match(
            [this](iroha::expected::Result<
                   std::unique_ptr<shared_model::interface::Peer>,
                   std::string>::ValueType &result) {
              this->this_peer_ = std::move(result.value);
            },
            [](const iroha::expected::Result<
                std::unique_ptr<shared_model::interface::Peer>,
                std::string>::ErrorType &error) {
              BOOST_THROW_EXCEPTION(std::runtime_error(
                  "Failed to create peer object for current irohad instance. "
                  + error.error));
            });

    iroha_instance_->initPipeline(keypair, maximum_proposal_size_);
    log_->info("created pipeline");
    iroha_instance_->getIrohaInstance()->resetOrderingService();

    makeFakePeers();
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
          verified_proposal_queue_.push(verified_proposal_and_errors);
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
    iroha_instance_->getIrohaInstance()->getStatusBus()->statuses().subscribe(
        [this](auto response) {
          responses_queues_[response->transactionHash().hex()].push(response);
          log_->info("response");
          queue_cond.notify_all();
        });

    if (fake_peers_.size() > 0) {
      log_->info("starting fake iroha peers");
      for (auto &fake_peer : fake_peers_) {
        fake_peer->run();
      }
    }
    // start instance
    log_->info("starting main iroha instance");
    iroha_instance_->run();
  }

  rxcpp::observable<std::shared_ptr<iroha::MstState>>
  IntegrationTestFramework::getMstStateUpdateObservable() {
    return iroha_instance_->getIrohaInstance()
        ->getMstProcessor()
        ->onStateUpdate();
  }

  rxcpp::observable<iroha::BatchPtr>
  IntegrationTestFramework::getMstPreparedBatchesObservable() {
    return iroha_instance_->getIrohaInstance()
        ->getMstProcessor()
        ->onPreparedBatches();
  }

  rxcpp::observable<iroha::BatchPtr>
  IntegrationTestFramework::getMstExpiredBatchesObservable() {
    return iroha_instance_->getIrohaInstance()
        ->getMstProcessor()
        ->onExpiredBatches();
  }

  rxcpp::observable<iroha::network::Commit>
  IntegrationTestFramework::getYacOnCommitObservable() {
    return iroha_instance_->getIrohaInstance()->getConsensusGate()->on_commit();
  }

  IntegrationTestFramework &
  IntegrationTestFramework::subscribeForAllMstNotifications(
      std::shared_ptr<iroha::network::MstTransportNotification> notification) {
    std::for_each(fake_peers_.cbegin(),
                  fake_peers_.cend(),
                  [&notification](const auto &fake_peer) {
                    fake_peer->subscribeForMstNotifications(notification);
                  });
    return *this;
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

  IntegrationTestFramework &IntegrationTestFramework::sendTxWithoutValidation(
      const shared_model::proto::Transaction &tx) {
    log_->info("sending transaction");
    log_->debug(tx.toString());

    command_client_.Torii(tx.getTransport());
    return *this;
  }

  IntegrationTestFramework &IntegrationTestFramework::sendTx(
      const shared_model::proto::Transaction &tx,
      std::function<void(const shared_model::proto::TransactionResponse &)>
          validation) {
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

    sendTxWithoutValidation(tx);
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
      if (!status.statelessErrorOrCommandName().empty()) {
        log_->debug("Got error while sending transaction: "
                    + status.statelessErrorOrCommandName());
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

  IntegrationTestFramework &IntegrationTestFramework::sendMstState(
      const shared_model::crypto::PublicKey &src_key,
      const iroha::MstState &mst_state) {
    iroha::network::sendStateAsync(
        *this_peer_, mst_state, src_key, *async_call_);
    return *this;
  }

  IntegrationTestFramework &IntegrationTestFramework::sendYacState(
      const std::vector<iroha::consensus::yac::VoteMessage> &yac_state) {
    yac_transport_->sendState(*this_peer_, yac_state);
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
    VerifiedProposalType verified_proposal_and_errors;
    fetchFromQueue(verified_proposal_queue_,
                   verified_proposal_and_errors,
                   proposal_waiting,
                   "missed verified proposal");
    ProposalType verified_proposal =
        std::move(verified_proposal_and_errors->verified_proposal);
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

  size_t IntegrationTestFramework::internalPort() const {
    return internal_port_;
  }

  void IntegrationTestFramework::done() {
    log_->info("done");
    cleanup();
  }

  void IntegrationTestFramework::cleanup() {
    log_->info("removing storage");
    if (iroha_instance_->getIrohaInstance()
        and iroha_instance_->getIrohaInstance()->storage) {
      iroha_instance_->getIrohaInstance()->storage->dropStorage();
      boost::filesystem::remove_all(iroha_instance_->block_store_dir_);
    }
  }
}  // namespace integration_framework
