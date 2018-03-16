/*
Copyright Soramitsu Co., Ltd. 2017 All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "model/converters/pb_transaction_factory.hpp"
#include "model/sha3_hash.hpp"
#include "module/irohad/ametsuchi/ametsuchi_mocks.hpp"
#include "module/irohad/network/network_mocks.hpp"

#include "endpoint.pb.h"
#include "main/server_runner.hpp"

#include "torii/command_client.hpp"

#include "torii/command_service.hpp"
#include "torii/processor/transaction_processor_impl.hpp"

#include "backend/protobuf/from_old_model.hpp"
#include "builders/protobuf/block.hpp"
#include "builders/protobuf/transaction.hpp"

#include "module/shared_model/builders/protobuf/test_block_builder.hpp"
#include "module/shared_model/builders/protobuf/test_proposal_builder.hpp"

constexpr const char *Ip = "0.0.0.0";
constexpr int Port = 50051;
constexpr size_t TimesToriiBlocking = 5;

using ::testing::_;
using ::testing::A;
using ::testing::AtLeast;
using ::testing::Return;

using namespace iroha::network;
using namespace iroha::ametsuchi;

using namespace std::chrono_literals;
constexpr std::chrono::milliseconds proposal_delay = 10s;

using iroha::Commit;

class CustomPeerCommunicationServiceMock : public PeerCommunicationService {
 public:
  CustomPeerCommunicationServiceMock(
      rxcpp::subjects::subject<
          std::shared_ptr<shared_model::interface::Proposal>> prop_notifier,
      rxcpp::subjects::subject<Commit> commit_notifier)
      : prop_notifier_(prop_notifier), commit_notifier_(commit_notifier){};

  void propagate_transaction(
      std::shared_ptr<const shared_model::interface::Transaction> transaction)
      override {}

  rxcpp::observable<std::shared_ptr<shared_model::interface::Proposal>>
  on_proposal() const override {
    return prop_notifier_.get_observable();
  }
  rxcpp::observable<Commit> on_commit() const override {
    return commit_notifier_.get_observable();
  }

 private:
  rxcpp::subjects::subject<std::shared_ptr<shared_model::interface::Proposal>>
      prop_notifier_;
  rxcpp::subjects::subject<Commit> commit_notifier_;
};

// TODO: allow dynamic port binding in ServerRunner IR-741
class ToriiServiceTest : public testing::Test {
 public:
  virtual void SetUp() {
    runner = new ServerRunner(std::string(Ip) + ":" + std::to_string(Port));

    // ----------- Command Service --------------
    pcsMock = std::make_shared<CustomPeerCommunicationServiceMock>(
        prop_notifier_, commit_notifier_);
    wsv_query = std::make_shared<MockWsvQuery>();
    block_query = std::make_shared<MockBlockQuery>();

    auto tx_processor =
        std::make_shared<iroha::torii::TransactionProcessorImpl>(pcsMock);
    auto pb_tx_factory =
        std::make_shared<iroha::model::converters::PbTransactionFactory>();

    EXPECT_CALL(*block_query, getTxByHashSync(_))
        .WillRepeatedly(Return(boost::none));

    //----------- Server run ----------------
    runner
        ->append(std::make_unique<torii::CommandService>(
            tx_processor, block_query, proposal_delay))
        .run();


    runner->waitForServersReady();
  }

  virtual void TearDown() {
    delete runner;
  }

  ServerRunner *runner;

  std::shared_ptr<MockWsvQuery> wsv_query;
  std::shared_ptr<MockBlockQuery> block_query;

  rxcpp::subjects::subject<std::shared_ptr<shared_model::interface::Proposal>>
      prop_notifier_;
  rxcpp::subjects::subject<Commit> commit_notifier_;

  std::shared_ptr<CustomPeerCommunicationServiceMock> pcsMock;

  shared_model::crypto::Keypair keypair =
      shared_model::crypto::DefaultCryptoAlgorithmType::generateKeypair();
};

/**
 * @given chain of CommandClient copies and moves
 * @when status is asked
 * @then grpc returns ok
 */
TEST_F(ToriiServiceTest, CommandClient) {
  iroha::protocol::TxStatusRequest tx_request;
  tx_request.set_tx_hash(std::string('1', 32));
  iroha::protocol::ToriiResponse toriiResponse;

  auto client1 = torii::CommandSyncClient(Ip, Port);
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
  std::vector<iroha::model::Transaction> txs;
  std::vector<std::string> tx_hashes;

  iroha::model::converters::PbTransactionFactory tx_factory;

  // create transactions, but do not send them
  for (size_t i = 0; i < TimesToriiBlocking; ++i) {
    auto new_tx = iroha::protocol::Transaction();
    auto payload = new_tx.mutable_payload();
    payload->set_tx_counter(i);
    payload->set_creator_account_id("accountA");

    auto iroha_tx = tx_factory.deserialize(new_tx);
    txs.push_back(*iroha_tx);
    auto tx_hash = iroha::hash(*iroha_tx);
    tx_hashes.push_back(tx_hash.to_string());
  }

  // get statuses of unsent transactions
  auto client = torii::CommandSyncClient(Ip, Port);

  for (size_t i = 0; i < TimesToriiBlocking; ++i) {
    iroha::protocol::TxStatusRequest tx_request;
    tx_request.set_tx_hash(tx_hashes.at(i));
    iroha::protocol::ToriiResponse toriiResponse;
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
  std::vector<std::string> tx_hashes;

  auto client1 = torii::CommandSyncClient(Ip, Port);

  // create transactions and send them to Torii
  std::string account_id = "some@account";
  for (size_t i = 0; i < TimesToriiBlocking; ++i) {
    auto shm_tx = shared_model::proto::TransactionBuilder()
                      .creatorAccountId(account_id)
                      .txCounter(i + 1)
                      .createdTime(iroha::time::now())
                      .setAccountQuorum(account_id, 2)
                      .build()
                      .signAndAddSignature(
                          shared_model::crypto::DefaultCryptoAlgorithmType::
                              generateKeypair());
    auto new_tx = shm_tx.getTransport();

    auto stat = client1.Torii(new_tx);
    txs.push_back(shm_tx);
    auto tx_hash = shared_model::crypto::toBinaryString(shm_tx.hash());
    tx_hashes.push_back(tx_hash);

    ASSERT_TRUE(stat.ok());
  }

  // create proposal from these transactions
  auto proposal = std::make_shared<shared_model::proto::Proposal>(
      TestProposalBuilder().transactions(txs).build());
  prop_notifier_.get_subscriber().on_next(proposal);

  torii::CommandSyncClient client2(client1);

  // check if stateless validation passed
  for (size_t i = 0; i < TimesToriiBlocking; ++i) {
    iroha::protocol::TxStatusRequest tx_request;
    tx_request.set_tx_hash(tx_hashes.at(i));
    iroha::protocol::ToriiResponse toriiResponse;
    client2.Status(tx_request, toriiResponse);

    ASSERT_EQ(toriiResponse.tx_status(),
              iroha::protocol::TxStatus::STATELESS_VALIDATION_SUCCESS);
  }

  // create block from the all transactions but the last one
  txs.pop_back();
  auto block = std::make_shared<shared_model::proto::Block>(
      TestBlockBuilder()
          .transactions(txs)
          .height(1)
          .createdTime(0)
          .prevHash(shared_model::crypto::Hash(std::string(32, '0')))
          .build());

  // create commit from block notifier's observable
  rxcpp::subjects::subject<std::shared_ptr<shared_model::interface::Block>>
      block_notifier_;
  Commit commit = block_notifier_.get_observable();

  // invoke on next of commit_notifier by sending new block to commit
  commit_notifier_.get_subscriber().on_next(commit);
  block_notifier_.get_subscriber().on_next(block);

  auto client3 = client2;
  // check if all transactions but the last one passed stateful validation
  for (size_t i = 0; i < TimesToriiBlocking - 1; ++i) {
    iroha::protocol::TxStatusRequest tx_request;
    tx_request.set_tx_hash(tx_hashes.at(i));
    iroha::protocol::ToriiResponse toriiResponse;
    client3.Status(tx_request, toriiResponse);

    ASSERT_EQ(toriiResponse.tx_status(),
              iroha::protocol::TxStatus::STATEFUL_VALIDATION_SUCCESS);
  }

  // end current commit
  block_notifier_.get_subscriber().on_completed();

  auto client4 = client3;
  // check if all transactions but the last have committed state
  for (size_t i = 0; i < TimesToriiBlocking - 1; ++i) {
    iroha::protocol::TxStatusRequest tx_request;
    tx_request.set_tx_hash(tx_hashes.at(i));
    iroha::protocol::ToriiResponse toriiResponse;
    client4.Status(tx_request, toriiResponse);

    ASSERT_EQ(toriiResponse.tx_status(), iroha::protocol::TxStatus::COMMITTED);
  }

  torii::CommandSyncClient client5(client4);
  // check if the last transaction from txs has failed stateful validation
  iroha::protocol::TxStatusRequest last_tx_request;
  last_tx_request.set_tx_hash(tx_hashes.at(TimesToriiBlocking - 1));
  iroha::protocol::ToriiResponse stful_invalid_response;
  client5.Status(last_tx_request, stful_invalid_response);
  ASSERT_EQ(stful_invalid_response.tx_status(),
            iroha::protocol::TxStatus::STATEFUL_VALIDATION_FAILED);
}

/**
 * @given torii service and some number of transactions with hashes
 * @when sending request on this txs
 * @then ensure that response has the same hash as sent tx
 */
TEST_F(ToriiServiceTest, CheckHash) {
  // given
  std::vector<std::string> tx_hashes;
  const int tx_num = 10;

  iroha::model::converters::PbTransactionFactory tx_factory;

  // create transactions, but do not send them
  for (size_t i = 0; i < tx_num; ++i) {
    auto new_tx = iroha::protocol::Transaction();
    auto payload = new_tx.mutable_payload();
    payload->set_tx_counter(i);
    auto tx = tx_factory.deserialize(new_tx);

    tx_hashes.push_back(iroha::hash(*tx).to_string());
  }

  auto client = torii::CommandSyncClient(Ip, Port);

  // get statuses of transactions
  for (auto &hash : tx_hashes) {
    iroha::protocol::TxStatusRequest tx_request;
    tx_request.set_tx_hash(hash);
    iroha::protocol::ToriiResponse toriiResponse;
    // when
    client.Status(tx_request, toriiResponse);
    // then
    ASSERT_EQ(toriiResponse.tx_hash(), hash);
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

  iroha::model::converters::PbTransactionFactory tx_factory;
  auto client = torii::CommandSyncClient(Ip, Port);

  auto new_tx = iroha::protocol::Transaction();
  auto payload = new_tx.mutable_payload();
  payload->set_tx_counter(1);
  payload->set_creator_account_id("accountA");

  auto iroha_tx = proto::TransactionBuilder()
                      .txCounter(1)
                      .creatorAccountId("a@domain")
                      .setAccountQuorum("a@domain", 2)
                      .createdTime(iroha::time::now())
                      .build()
                      .signAndAddSignature(keypair);

  std::string txhash = crypto::toBinaryString(iroha_tx.hash());

  std::vector<iroha::protocol::ToriiResponse> torii_response;
  // StatusStream is a blocking call and returns only when the last status
  // (Committed in this case) will be received. We start request before
  // transaction sending so we need in a separate thread for it.
  std::thread t([&] {
    iroha::protocol::TxStatusRequest tx_request;
    tx_request.set_tx_hash(txhash);
    client.StatusStream(tx_request, torii_response);
  });

  client.Torii(new_tx);

  std::vector<decltype(iroha_tx)> txs;
  txs.push_back(iroha_tx);
  auto proposal =
      std::make_shared<proto::Proposal>(proto::ProposalBuilder()
                                            .createdTime(iroha::time::now())
                                            .transactions(txs)
                                            .height(1)
                                            .build());
  prop_notifier_.get_subscriber().on_next(proposal);

  //  iroha::model::Block block;
  auto block = std::make_shared<proto::Block>(
      proto::BlockBuilder()
          .height(1)
          .createdTime(iroha::time::now())
          .transactions(txs)
          .prevHash(crypto::Hash(std::string(32, '0')))
          .build()
          .signAndAddSignature(keypair));

  // create commit from block notifier's observable
  rxcpp::subjects::subject<std::shared_ptr<shared_model::interface::Block>>
      block_notifier_;
  Commit commit = block_notifier_.get_observable();

  // invoke on next of commit_notifier by sending new block to commit
  commit_notifier_.get_subscriber().on_next(commit);
  block_notifier_.get_subscriber().on_next(block);

  block_notifier_.get_subscriber().on_completed();
  t.join();

  ASSERT_GE(torii_response.size(), 3);
  ASSERT_EQ(torii_response.back().tx_status(),
            iroha::protocol::TxStatus::COMMITTED);
}

/**
 * @given torii service and fake transaction hash
 * @when sending streaming request for this hash
 * @then ensure that response will have exactly 1 status - NOT_RECEIVED
 */
TEST_F(ToriiServiceTest, StreamingNoTx) {
  auto client = torii::CommandSyncClient(Ip, Port);
  std::vector<iroha::protocol::ToriiResponse> torii_response;
  std::thread t([&]() {
    iroha::protocol::TxStatusRequest tx_request;
    tx_request.set_tx_hash("0123456789abcdef");
    client.StatusStream(tx_request, torii_response);
  });

  t.join();
  ASSERT_EQ(torii_response.size(), 1);
  ASSERT_EQ(torii_response.at(0).tx_status(),
            iroha::protocol::TxStatus::NOT_RECEIVED);
}
