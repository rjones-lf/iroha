/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "builders/protobuf/block.hpp"
#include "builders/protobuf/proposal.hpp"
#include "builders/protobuf/transaction.hpp"
#include "endpoint.pb.h"
#include "main/server_runner.hpp"
#include "module/irohad/ametsuchi/ametsuchi_mocks.hpp"
#include "module/irohad/multi_sig_transactions/mst_mocks.hpp"
#include "module/irohad/network/network_mocks.hpp"
#include "module/shared_model/builders/protobuf/test_block_builder.hpp"
#include "module/shared_model/builders/protobuf/test_proposal_builder.hpp"
#include "module/shared_model/builders/protobuf/test_query_response_builder.hpp"
#include "module/shared_model/builders/protobuf/test_transaction_builder.hpp"
#include "torii/command_client.hpp"
#include "torii/command_service.hpp"
#include "torii/impl/status_bus_impl.hpp"
#include "torii/processor/transaction_processor_impl.hpp"

constexpr size_t TimesToriiBlocking = 5;

using ::testing::_;
using ::testing::A;
using ::testing::AtLeast;
using ::testing::Return;

using namespace iroha::network;
using namespace iroha::ametsuchi;
using namespace iroha::torii;
using namespace iroha::synchronizer;

using namespace std::chrono_literals;
constexpr std::chrono::milliseconds initial_timeout = 1s;
constexpr std::chrono::milliseconds nonfinal_timeout = 2 * 10s;

/**
The do-while cycle imitates client resubscription to the stream. Stream
"expiration" is a valid designed case (see pr #1615 for the details).

The number of attempts (3) is a magic constant here. The idea behind this number
is the following: only one resubscription is usually enough to pass the test; if
three resubscribes were not enough, then most likely there is another bug.
 */
constexpr uint32_t resubscribe_attempts = 3;

class CustomPeerCommunicationServiceMock : public PeerCommunicationService {
 public:
  CustomPeerCommunicationServiceMock(
      rxcpp::subjects::subject<
          std::shared_ptr<shared_model::interface::Proposal>> prop_notifier,
      rxcpp::subjects::subject<SynchronizationEvent> commit_notifier,
      rxcpp::subjects::subject<
          std::shared_ptr<iroha::validation::VerifiedProposalAndErrors>>
          verified_prop_notifier)
      : prop_notifier_(prop_notifier),
        commit_notifier_(commit_notifier),
        verified_prop_notifier_(verified_prop_notifier){};

  void propagate_transaction(
      std::shared_ptr<const shared_model::interface::Transaction> transaction)
      const override {}

  void propagate_batch(
      const shared_model::interface::TransactionBatch &batch) const override {}

  rxcpp::observable<std::shared_ptr<shared_model::interface::Proposal>>
  on_proposal() const override {
    return prop_notifier_.get_observable();
  }
  rxcpp::observable<SynchronizationEvent> on_commit() const override {
    return commit_notifier_.get_observable();
  }

  rxcpp::observable<
      std::shared_ptr<iroha::validation::VerifiedProposalAndErrors>>
  on_verified_proposal() const override {
    return verified_prop_notifier_.get_observable();
  };

 private:
  rxcpp::subjects::subject<std::shared_ptr<shared_model::interface::Proposal>>
      prop_notifier_;
  rxcpp::subjects::subject<SynchronizationEvent> commit_notifier_;
  rxcpp::subjects::subject<
      std::shared_ptr<iroha::validation::VerifiedProposalAndErrors>>
      verified_prop_notifier_;
};

class ToriiServiceTest : public testing::Test {
 public:
  virtual void SetUp() {
    runner = std::make_unique<ServerRunner>(ip + ":0");

    // ----------- Command Service --------------
    pcsMock = std::make_shared<CustomPeerCommunicationServiceMock>(
        prop_notifier_, commit_notifier_, verified_prop_notifier_);
    mst = std::make_shared<iroha::MockMstProcessor>();
    wsv_query = std::make_shared<MockWsvQuery>();
    block_query = std::make_shared<MockBlockQuery>();
    storage = std::make_shared<MockStorage>();

    EXPECT_CALL(*mst, onPreparedBatchesImpl())
        .WillRepeatedly(Return(mst_prepared_notifier.get_observable()));
    EXPECT_CALL(*mst, onExpiredBatchesImpl())
        .WillRepeatedly(Return(mst_expired_notifier.get_observable()));

    auto status_bus = std::make_shared<iroha::torii::StatusBusImpl>();
    auto tx_processor =
        std::make_shared<iroha::torii::TransactionProcessorImpl>(
            pcsMock, mst, status_bus);

    EXPECT_CALL(*block_query, getTxByHashSync(_))
        .WillRepeatedly(Return(boost::none));
    EXPECT_CALL(*storage, getBlockQuery()).WillRepeatedly(Return(block_query));

    //----------- Server run ----------------
    runner
        ->append(std::make_unique<torii::CommandService>(tx_processor,
                                                         storage,
                                                         status_bus,
                                                         initial_timeout,
                                                         nonfinal_timeout))
        .run()
        .match(
            [this](iroha::expected::Value<int> port) {
              this->port = port.value;
            },
            [](iroha::expected::Error<std::string> err) {
              FAIL() << err.error;
            });

    runner->waitForServersReady();
  }

  std::unique_ptr<ServerRunner> runner;

  std::shared_ptr<MockWsvQuery> wsv_query;
  std::shared_ptr<MockBlockQuery> block_query;
  std::shared_ptr<MockStorage> storage;

  rxcpp::subjects::subject<std::shared_ptr<shared_model::interface::Proposal>>
      prop_notifier_;
  rxcpp::subjects::subject<SynchronizationEvent> commit_notifier_;
  rxcpp::subjects::subject<
      std::shared_ptr<iroha::validation::VerifiedProposalAndErrors>>
      verified_prop_notifier_;
  rxcpp::subjects::subject<iroha::DataType> mst_prepared_notifier;
  rxcpp::subjects::subject<iroha::DataType> mst_expired_notifier;

  std::shared_ptr<CustomPeerCommunicationServiceMock> pcsMock;
  std::shared_ptr<iroha::MockMstProcessor> mst;

  shared_model::crypto::Keypair keypair =
      shared_model::crypto::DefaultCryptoAlgorithmType::generateKeypair();

  const std::string ip = "127.0.0.1";
  int port;
};

/**
 * @given chain of CommandClient copies and moves
 * @when status is asked
 * @then grpc returns ok
 */
TEST_F(ToriiServiceTest, CommandClient) {
  iroha::protocol::TxStatusRequest tx_request;
  tx_request.set_tx_hash(std::string(32, '1'));
  iroha::protocol::ToriiResponse toriiResponse;

  auto client1 = torii::CommandSyncClient(ip, port);
  // Copy ctor
  torii::CommandSyncClient client2(client1);
  // copy assignment
  auto client3 = client2;
  // move ctor
  torii::CommandSyncClient client4(std::move(client3));
  // move assignment
  auto client5 = std::move(client4);
  auto stat = client5.Status(tx_request, toriiResponse);

  ASSERT_TRUE(stat.ok());
}
/**
 * @given torii service and number of transactions
 * @when retrieving their status
 * @then ensure those are not received
 */
TEST_F(ToriiServiceTest, StatusWhenTxWasNotReceivedBlocking) {
  std::vector<shared_model::proto::Transaction> txs;
  std::vector<shared_model::interface::types::HashType> tx_hashes;

  // create transactions, but do not send them
  for (size_t i = 0; i < TimesToriiBlocking; ++i) {
    auto tx = TestTransactionBuilder().creatorAccountId("accountA").build();
    txs.push_back(tx);
    tx_hashes.push_back(tx.hash());
  }

  // get statuses of unsent transactions
  auto client = torii::CommandSyncClient(ip, port);

  for (size_t i = 0; i < TimesToriiBlocking; ++i) {
    iroha::protocol::TxStatusRequest tx_request;
    tx_request.set_tx_hash(
        shared_model::crypto::toBinaryString(tx_hashes.at(i)));
    iroha::protocol::ToriiResponse toriiResponse;
    // this test does not require the fix for thread scheduling issues
    client.Status(tx_request, toriiResponse);
    ASSERT_EQ(toriiResponse.tx_status(),
              iroha::protocol::TxStatus::NOT_RECEIVED);
  }
}

/**
 * That test simulates the real behavior of the blocking Torii.
 *
 * @given torii service with mocked CommunicationService
 *        that is subscribed on custom rxcpp::subjects
 * @when sending some number of transactions to the Torii
 * @then ensure it perform as real one:
 *       - Torii returns ok status
 *       - Proper txs responses in Status are STATELESS_VALIDATION_SUCCESS,
 *         then STATEFUL_VALIDATION_SUCCESS and COMMITTED (order matters)
 *       - Tx that not in a block returns STATELESS_VALIDATION_SUCCESS and
           then STATEFUL_VALIDATION_FAILED
 */
TEST_F(ToriiServiceTest, StatusWhenBlocking) {
  std::vector<shared_model::proto::Transaction> txs;
  std::vector<shared_model::interface::types::HashType> tx_hashes;

  auto client1 = torii::CommandSyncClient(ip, port);

  // create transactions and send them to Torii
  std::string account_id = "some@account";
  auto now = iroha::time::now();
  for (size_t i = 0; i < TimesToriiBlocking; ++i) {
    auto shm_tx = shared_model::proto::TransactionBuilder()
                      .creatorAccountId(account_id)
                      .createdTime(now + i)
                      .setAccountQuorum(account_id, 2)
                      .quorum(1)
                      .build()
                      .signAndAddSignature(
                          shared_model::crypto::DefaultCryptoAlgorithmType::
                              generateKeypair())
                      .finish();
    const auto &new_tx = shm_tx.getTransport();

    auto stat = client1.Torii(new_tx);
    txs.push_back(shm_tx);
    tx_hashes.push_back(shm_tx.hash());

    ASSERT_TRUE(stat.ok());
  }

  // create proposal from these transactions
  auto proposal = std::make_shared<shared_model::proto::Proposal>(
      TestProposalBuilder()
          .height(1)
          .createdTime(iroha::time::now())
          .transactions(txs)
          .build());
  prop_notifier_.get_subscriber().on_next(proposal);

  torii::CommandSyncClient client2(client1);

  // check if stateless validation passed
  for (size_t i = 0; i < TimesToriiBlocking; ++i) {
    iroha::protocol::TxStatusRequest tx_request;
    tx_request.set_tx_hash(
        shared_model::crypto::toBinaryString(tx_hashes.at(i)));
    iroha::protocol::ToriiResponse toriiResponse;
    client2.Status(tx_request, toriiResponse);

    ASSERT_EQ(toriiResponse.tx_status(),
              iroha::protocol::TxStatus::STATELESS_VALIDATION_SUCCESS);
  }

  // create block from the all transactions but the last one
  auto failed_tx_hash = txs.back().hash();
  txs.pop_back();
  auto block =
      clone(TestBlockBuilder()
                .transactions(txs)
                .height(1)
                .createdTime(0)
                .prevHash(shared_model::crypto::Hash(std::string(32, '0')))
                .build());

  // notify the verified proposal event about txs, which passed stateful
  // validation
  auto verified_proposal = std::make_shared<shared_model::proto::Proposal>(
      TestProposalBuilder()
          .height(1)
          .createdTime(iroha::time::now())
          .transactions(txs)
          .build());
  auto errors = iroha::validation::TransactionsErrors{std::make_pair(
      iroha::validation::CommandError{
          "FailedCommand", "stateful validation failed", true, 2},
      failed_tx_hash)};
  auto stringified_error = "Stateful validation error in transaction "
                           + failed_tx_hash.hex() + ": "
                           "command 'FailedCommand' with index '2' "
                           "did not pass verification with error 'stateful "
                           "validation failed'";
  verified_prop_notifier_.get_subscriber().on_next(
      std::make_shared<iroha::validation::VerifiedProposalAndErrors>(
          std::make_pair(verified_proposal, errors)));

  // create commit from block notifier's observable
  rxcpp::subjects::subject<std::shared_ptr<shared_model::interface::Block>>
      block_notifier_;
  SynchronizationEvent commit{block_notifier_.get_observable(),
                              SynchronizationOutcomeType::kCommit};

  // invoke on next of commit_notifier by sending new block to commit
  commit_notifier_.get_subscriber().on_next(commit);
  block_notifier_.get_subscriber().on_next(std::move(block));

  auto client3 = client2;
  // check if all transactions but the last one passed stateful validation
  for (size_t i = 0; i < TimesToriiBlocking - 1; ++i) {
    iroha::protocol::TxStatusRequest tx_request;
    tx_request.set_tx_hash(
        shared_model::crypto::toBinaryString(tx_hashes.at(i)));
    iroha::protocol::ToriiResponse toriiResponse;

    auto resub_counter(resubscribe_attempts);
    do {
      client3.Status(tx_request, toriiResponse);
    } while (toriiResponse.tx_status()
                 != iroha::protocol::TxStatus::STATEFUL_VALIDATION_SUCCESS
             and --resub_counter);

    ASSERT_EQ(toriiResponse.tx_status(),
              iroha::protocol::TxStatus::STATEFUL_VALIDATION_SUCCESS);
  }

  // end current commit
  block_notifier_.get_subscriber().on_completed();

  auto client4 = client3;
  // check if all transactions but the last have committed state
  for (size_t i = 0; i < TimesToriiBlocking - 1; ++i) {
    iroha::protocol::TxStatusRequest tx_request;
    tx_request.set_tx_hash(
        shared_model::crypto::toBinaryString(tx_hashes.at(i)));
    iroha::protocol::ToriiResponse toriiResponse;
    client4.Status(tx_request, toriiResponse);

    ASSERT_EQ(toriiResponse.tx_status(), iroha::protocol::TxStatus::COMMITTED);
  }

  torii::CommandSyncClient client5(client4);
  // check if the last transaction from txs has failed stateful validation
  iroha::protocol::TxStatusRequest last_tx_request;
  last_tx_request.set_tx_hash(shared_model::crypto::toBinaryString(
      tx_hashes.at(TimesToriiBlocking - 1)));
  iroha::protocol::ToriiResponse stful_invalid_response;
  client5.Status(last_tx_request, stful_invalid_response);
  ASSERT_EQ(stful_invalid_response.tx_status(),
            iroha::protocol::TxStatus::STATEFUL_VALIDATION_FAILED);
  ASSERT_EQ(stful_invalid_response.error_message(), stringified_error);
}

/**
 * @given torii service and some number of transactions with hashes
 * @when sending request on this txs
 * @then ensure that response has the same hash as sent tx
 */
TEST_F(ToriiServiceTest, CheckHash) {
  // given
  std::vector<shared_model::interface::types::HashType> tx_hashes;
  const int tx_num = 10;

  // create transactions, but do not send them
  for (size_t i = 0; i < tx_num; ++i) {
    auto tx = TestTransactionBuilder().build();
    tx_hashes.push_back(tx.hash());
  }

  auto client = torii::CommandSyncClient(ip, port);

  // get statuses of transactions
  for (auto &hash : tx_hashes) {
    iroha::protocol::TxStatusRequest tx_request;
    tx_request.set_tx_hash(shared_model::crypto::toBinaryString(hash));
    iroha::protocol::ToriiResponse toriiResponse;
    const auto binary_hash = shared_model::crypto::toBinaryString(hash);
    auto resub_counter(resubscribe_attempts);
    do {
      client.Status(tx_request, toriiResponse);
    } while (toriiResponse.tx_hash() != binary_hash and --resub_counter);
    ASSERT_EQ(toriiResponse.tx_hash(), binary_hash);
  }
}

/**
 * @given torii service and one valid transaction
 * @when starting StatusStream and then sending transaction to Iroha
 * @then ensure that response will have at least 3 statuses
 * (it should contain STATELESS_VALIDATION_SUCCESS, STATEFUL_VALIDATION_SUCCESS
 * and COMMITTED) and the last status should be COMMITTED
 */
TEST_F(ToriiServiceTest, StreamingFullPipelineTest) {
  using namespace shared_model;

  auto client = torii::CommandSyncClient(ip, port);
  auto iroha_tx = proto::TransactionBuilder()
                      .creatorAccountId("a@domain")
                      .setAccountQuorum("a@domain", 2)
                      .createdTime(iroha::time::now())
                      .quorum(1)
                      .build()
                      .signAndAddSignature(keypair)
                      .finish();

  std::string txhash = crypto::toBinaryString(iroha_tx.hash());

  std::vector<iroha::protocol::ToriiResponse> torii_response;
  // StatusStream is a blocking call and returns only when the last status
  // (Committed in this case) will be received. We start request before
  // transaction sending so we need in a separate thread for it.
  std::thread t([&] {
    auto resub_counter(resubscribe_attempts);
    iroha::protocol::TxStatusRequest tx_request;
    tx_request.set_tx_hash(txhash);
    do {
      client.StatusStream(tx_request, torii_response);
    } while (torii_response.back().tx_status()
                 != iroha::protocol::TxStatus::COMMITTED
             and --resub_counter);
  });

  client.Torii(iroha_tx.getTransport());

  std::vector<decltype(iroha_tx)> txs;
  txs.push_back(iroha_tx);
  auto proposal =
      std::make_shared<proto::Proposal>(proto::ProposalBuilder()
                                            .createdTime(iroha::time::now())
                                            .transactions(txs)
                                            .height(1)
                                            .build());
  prop_notifier_.get_subscriber().on_next(proposal);
  verified_prop_notifier_.get_subscriber().on_next(
      std::make_shared<iroha::validation::VerifiedProposalAndErrors>(
          std::make_pair(proposal, iroha::validation::TransactionsErrors{})));

  auto block = clone(proto::BlockBuilder()
                         .height(1)
                         .createdTime(iroha::time::now())
                         .transactions(txs)
                         .prevHash(crypto::Hash(std::string(32, '0')))
                         .build()
                         .signAndAddSignature(keypair)
                         .finish());

  // create commit from block notifier's observable
  rxcpp::subjects::subject<std::shared_ptr<shared_model::interface::Block>>
      block_notifier_;
  SynchronizationEvent commit{block_notifier_.get_observable(),
                              SynchronizationOutcomeType::kCommit};

  // invoke on next of commit_notifier by sending new block to commit
  commit_notifier_.get_subscriber().on_next(commit);
  block_notifier_.get_subscriber().on_next(std::move(block));

  block_notifier_.get_subscriber().on_completed();
  t.join();

  // we can be sure only about final status
  // it can be only one or to follow by some non-final
  ASSERT_EQ(torii_response.back().tx_status(),
            iroha::protocol::TxStatus::COMMITTED);
}

/**
 * @given torii service and fake transaction hash
 * @when sending streaming request for this hash
 * @then ensure that response will have exactly 1 status - NOT_RECEIVED
 */
TEST_F(ToriiServiceTest, StreamingNoTx) {
  auto client = torii::CommandSyncClient(ip, port);
  std::vector<iroha::protocol::ToriiResponse> torii_response;
  std::thread t([&]() {
    iroha::protocol::TxStatusRequest tx_request;
    tx_request.set_tx_hash("0123456789abcdef");
    // this test does not require the fix for thread scheduling issues
    client.StatusStream(tx_request, torii_response);
  });

  t.join();
  ASSERT_EQ(torii_response.size(), 1);
  ASSERT_EQ(torii_response.at(0).tx_status(),
            iroha::protocol::TxStatus::NOT_RECEIVED);
}

/**
 * Checks that torii is able to handle lists (sequences) of transactions
 *
 * @given torii service and collection of transactions
 * @when that collection is asked to be processed by Torii
 * @then statuses of all transactions from that request are STATELESS_VALID
 */
TEST_F(ToriiServiceTest, ListOfTxs) {
  const auto test_txs_number = 5;

  // initial preparations: creation of variables and txs
  auto client = torii::CommandSyncClient(ip, port);
  std::vector<shared_model::interface::types::HashType> tx_hashes(
      test_txs_number);
  iroha::protocol::TxList tx_list;

  for (auto i = 0; i < test_txs_number; ++i) {
    auto shm_tx = shared_model::proto::TransactionBuilder()
                      .creatorAccountId("doge@master" + std::to_string(i))
                      .createdTime(iroha::time::now())
                      .setAccountQuorum("doge@master", 2)
                      .quorum(1)
                      .build()
                      .signAndAddSignature(
                          shared_model::crypto::DefaultCryptoAlgorithmType::
                              generateKeypair())
                      .finish();
    tx_hashes[i] = shm_tx.hash();
    new (tx_list.add_transactions())
        iroha::protocol::Transaction(shm_tx.getTransport());
  }

  // send the txs
  client.ListTorii(tx_list);

  // check their statuses
  std::for_each(
      std::begin(tx_hashes), std::end(tx_hashes), [&client](auto &hash) {
        iroha::protocol::TxStatusRequest tx_request;
        tx_request.set_tx_hash(shared_model::crypto::toBinaryString(hash));
        iroha::protocol::ToriiResponse toriiResponse;

        auto resub_counter(resubscribe_attempts);
        do {
          client.Status(tx_request, toriiResponse);
        } while (toriiResponse.tx_status()
                     != iroha::protocol::TxStatus::STATELESS_VALIDATION_SUCCESS
                 and --resub_counter);

        ASSERT_EQ(toriiResponse.tx_status(),
                  iroha::protocol::TxStatus::STATELESS_VALIDATION_SUCCESS);
      });
}

/**
 * Checks that in case of failed transactions list every transaction has a
 * related status and error message
 *
 * @given torii service and a list of bad-formed transactions
 * @when checking those transactions after sending them to Torii
 * @then every transaction from this list will have STATELESS_FAILED status @and
 * the same corresponding error message
 */
TEST_F(ToriiServiceTest, FailedListOfTxs) {
  const auto test_txs_number = 5;

  // initial preparations: creation of variables and txs
  auto client = torii::CommandSyncClient(ip, port);
  std::vector<shared_model::interface::types::HashType> tx_hashes(
      test_txs_number);
  iroha::protocol::TxList tx_list;

  for (auto i = 0; i < test_txs_number; ++i) {
    auto shm_tx = TestTransactionBuilder()
                      .creatorAccountId("doge@master" + std::to_string(i))
                      .createdTime(iroha::time::now(std::chrono::hours(24)
                                                    + std::chrono::minutes(1)))
                      .setAccountQuorum("doge@master", 2)
                      .quorum(1)
                      .build();
    tx_hashes[i] = shm_tx.hash();
    new (tx_list.add_transactions())
        iroha::protocol::Transaction(shm_tx.getTransport());
  }

  // send the txs
  client.ListTorii(tx_list);

  // actual error message is too big and hardly predictable, so we want at least
  // to make sure that edges of tx list are right
  auto error_msg_beginning =
      "Stateless invalid tx in transaction sequence, beginning "
      "with tx : "
      + tx_hashes.front().hex() + " and ending with tx "
      + tx_hashes.back().hex();

  // check their statuses
  std::for_each(
      std::begin(tx_hashes),
      std::end(tx_hashes),
      [&client, &error_msg_beginning](auto &hash) {
        iroha::protocol::TxStatusRequest tx_request;
        tx_request.set_tx_hash(shared_model::crypto::toBinaryString(hash));
        iroha::protocol::ToriiResponse toriiResponse;
        auto resub_counter(resubscribe_attempts);
        do {
          client.Status(tx_request, toriiResponse);
        } while (toriiResponse.tx_status()
                     != iroha::protocol::TxStatus::STATELESS_VALIDATION_FAILED
                 and --resub_counter);

        auto error_beginning = toriiResponse.error_message().substr(
            0, toriiResponse.error_message().find_first_of('.'));

        ASSERT_EQ(toriiResponse.tx_status(),
                  iroha::protocol::TxStatus::STATELESS_VALIDATION_FAILED);
        ASSERT_EQ(error_beginning, error_msg_beginning);
      });
}
