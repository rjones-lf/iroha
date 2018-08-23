/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <boost/range/join.hpp>
#include "builders/protobuf/common_objects/proto_signature_builder.hpp"
#include "builders/protobuf/proposal.hpp"
#include "builders/protobuf/transaction.hpp"
#include "framework/batch_helper.hpp"
#include "framework/specified_visitor.hpp"
#include "framework/test_subscriber.hpp"
#include "interfaces/iroha_internal/transaction_sequence.hpp"
#include "module/irohad/multi_sig_transactions/mst_mocks.hpp"
#include "module/irohad/network/network_mocks.hpp"
#include "module/irohad/torii/torii_mocks.hpp"
#include "module/shared_model/builders/protobuf/test_block_builder.hpp"
#include "module/shared_model/builders/protobuf/test_proposal_builder.hpp"
#include "module/shared_model/builders/protobuf/test_transaction_builder.hpp"
#include "torii/impl/status_bus_impl.hpp"
#include "torii/processor/transaction_processor_impl.hpp"

using namespace iroha;
using namespace iroha::network;
using namespace iroha::torii;
using namespace iroha::synchronizer;
using namespace framework::test_subscriber;

using ::testing::_;
using ::testing::A;
using ::testing::Return;

class TransactionProcessorTest : public ::testing::Test {
 public:
  void SetUp() override {
    pcs = std::make_shared<MockPeerCommunicationService>();
    mp = std::make_shared<MockMstProcessor>();

    EXPECT_CALL(*pcs, on_proposal())
        .WillRepeatedly(Return(prop_notifier.get_observable()));
    EXPECT_CALL(*pcs, on_commit())
        .WillRepeatedly(Return(commit_notifier.get_observable()));
    EXPECT_CALL(*pcs, on_verified_proposal())
        .WillRepeatedly(Return(verified_prop_notifier.get_observable()));

    EXPECT_CALL(*mp, onPreparedBatchesImpl())
        .WillRepeatedly(Return(mst_prepared_notifier.get_observable()));
    EXPECT_CALL(*mp, onExpiredBatchesImpl())
        .WillRepeatedly(Return(mst_expired_notifier.get_observable()));

    status_bus = std::make_shared<MockStatusBus>();
    tp = std::make_shared<TransactionProcessorImpl>(pcs, mp, status_bus);
  }

  auto base_tx() {
    return shared_model::proto::TransactionBuilder()
        .creatorAccountId("user@domain")
        .createdTime(iroha::time::now())
        .setAccountQuorum("user@domain", 2)
        .quorum(1);
  }

  auto baseTestTx() {
    return TestTransactionBuilder()
        .createdTime(iroha::time::now())
        .creatorAccountId("user@domain")
        .setAccountQuorum("user@domain", 2)
        .quorum(1)
        .build();
  }

  inline auto makeKey() {
    return shared_model::crypto::DefaultCryptoAlgorithmType::generateKeypair();
  }

  template <typename Transaction, typename... KeyPairs>
  auto addSignaturesFromKeyPairs(Transaction &&tx, KeyPairs... keypairs) {
    auto create_signature = [&](auto &&key_pair) {
      auto &payload = tx.payload();
      auto signedBlob = shared_model::crypto::CryptoSigner<>::sign(
          shared_model::crypto::Blob(payload), key_pair);
      tx.addSignature(signedBlob, key_pair.publicKey());
    };

    int temp[] = {(create_signature(std::forward<KeyPairs>(keypairs)), 0)...};
    (void)temp;

    return tx;
  }

 protected:
  using StatusMapType = std::unordered_map<
      shared_model::crypto::Hash,
      std::shared_ptr<shared_model::interface::TransactionResponse>,
      shared_model::crypto::Hash::Hasher>;

  /**
   * Checks if all transactions have corresponding status
   * @param transactions transactions to check status
   * @param status to be checked
   */
  template <typename Status>
  void validateStatuses(
      std::vector<shared_model::proto::Transaction> &transactions) {
    for (const auto &tx : transactions) {
      auto tx_status = status_map.find(tx.hash());
      ASSERT_NE(tx_status, status_map.end());
      ASSERT_NO_THROW(boost::apply_visitor(
          framework::SpecifiedVisitor<Status>(), tx_status->second->get()));
    }
  }

  rxcpp::subjects::subject<iroha::DataType> mst_prepared_notifier;
  rxcpp::subjects::subject<iroha::DataType> mst_expired_notifier;

  std::shared_ptr<MockPeerCommunicationService> pcs;
  std::shared_ptr<MockStatusBus> status_bus;
  std::shared_ptr<TransactionProcessorImpl> tp;
  std::shared_ptr<MockMstProcessor> mp;

  StatusMapType status_map;
  shared_model::builder::TransactionStatusBuilder<
      shared_model::proto::TransactionStatusBuilder>
      status_builder;

  rxcpp::subjects::subject<std::shared_ptr<shared_model::interface::Proposal>>
      prop_notifier;
  rxcpp::subjects::subject<SynchronizationEvent> commit_notifier;
  rxcpp::subjects::subject<
      std::shared_ptr<iroha::validation::VerifiedProposalAndErrors>>
      verified_prop_notifier;

  const size_t proposal_size = 5;
  const size_t block_size = 3;
};
/**
 * @given transaction processor
 * @when transactions passed to processor compose proposal which is sent to peer
 * communication service
 * @then for every transaction STATELESS_VALID status is returned
 */
TEST_F(TransactionProcessorTest, TransactionProcessorOnProposalTest) {
  std::vector<shared_model::proto::Transaction> txs;
  for (size_t i = 0; i < proposal_size; i++) {
    auto &&tx = addSignaturesFromKeyPairs(baseTestTx(), makeKey());
    txs.push_back(tx);
    status_map[tx.hash()] =
        status_builder.notReceived().txHash(tx.hash()).build();
  }

  EXPECT_CALL(*status_bus, publish(_))
      .Times(proposal_size)
      .WillRepeatedly(testing::Invoke([this](auto response) {
        status_map[response->transactionHash()] = response;
      }));

  EXPECT_CALL(*mp, propagateBatchImpl(_)).Times(0);
  EXPECT_CALL(*pcs, propagate_transaction(_)).Times(txs.size());

  for (const auto &tx : txs) {
    tp->batchHandle(framework::batch::createBatchFromSingleTransaction(
        std::shared_ptr<shared_model::interface::Transaction>(clone(tx))));
  }

  // create proposal and notify about it
  auto proposal = std::make_shared<shared_model::proto::Proposal>(
      TestProposalBuilder().transactions(txs).build());

  prop_notifier.get_subscriber().on_next(proposal);
  prop_notifier.get_subscriber().on_completed();

  SCOPED_TRACE("Stateless valid status verification");
  validateStatuses<shared_model::interface::StatelessValidTxResponse>(txs);
}

/**
 * @given transactions from the same batch
 * @when transactions sequence is created and propagated @and all transactions
 * were returned by pcs in proposal notifier
 * @then all transactions have stateless valid status
 */
TEST_F(TransactionProcessorTest, TransactionProcessorOnProposalBatchTest) {
  using namespace shared_model::validation;
  using TxsValidator = DefaultSignedTransactionsValidator;

  auto transactions =
      framework::batch::createValidBatch(proposal_size).transactions();

  EXPECT_CALL(*status_bus, publish(_))
      .Times(proposal_size)
      .WillRepeatedly(testing::Invoke([this](auto response) {
        status_map[response->transactionHash()] = response;
      }));

  auto transaction_sequence_result =
      shared_model::interface::TransactionSequence::createTransactionSequence(
          transactions, TxsValidator());
  auto transaction_sequence =
      framework::expected::val(transaction_sequence_result).value().value;

  EXPECT_CALL(*mp, propagateBatchImpl(_)).Times(0);
  EXPECT_CALL(*pcs, propagate_batch(_))
      .Times(transaction_sequence.batches().size());

  for (const auto &batch : transaction_sequence.batches()) {
    tp->batchHandle(batch);
  }

  // create proposal from sequence transactions and notify about it
  std::vector<shared_model::proto::Transaction> proto_transactions;

  std::transform(
      transactions.begin(),
      transactions.end(),
      std::back_inserter(proto_transactions),
      [](const auto tx) {
        return *std::static_pointer_cast<shared_model::proto::Transaction>(tx);
      });

  auto proposal = std::make_shared<shared_model::proto::Proposal>(
      TestProposalBuilder().transactions(proto_transactions).build());

  prop_notifier.get_subscriber().on_next(proposal);
  prop_notifier.get_subscriber().on_completed();

  SCOPED_TRACE("Stateless valid status verification");
  validateStatuses<shared_model::interface::StatelessValidTxResponse>(
      proto_transactions);
}

/**
 * @given transaction processor
 * @when transactions compose proposal which is sent to peer
 * communication service @and all transactions composed the block
 * @then for every transaction STATEFUL_VALID status is returned
 */
TEST_F(TransactionProcessorTest, TransactionProcessorBlockCreatedTest) {
  std::vector<shared_model::proto::Transaction> txs;
  for (size_t i = 0; i < proposal_size; i++) {
    auto &&tx = TestTransactionBuilder().createdTime(i).build();
    txs.push_back(tx);
    status_map[tx.hash()] =
        status_builder.notReceived().txHash(tx.hash()).build();
  }

  EXPECT_CALL(*status_bus, publish(_))
      .Times(txs.size() * 2)
      .WillRepeatedly(testing::Invoke([this](auto response) {
        status_map[response->transactionHash()] = response;
      }));

  EXPECT_CALL(*mp, propagateBatchImpl(_)).Times(0);
  EXPECT_CALL(*pcs, propagate_transaction(_)).Times(txs.size());

  for (const auto &tx : txs) {
    tp->batchHandle(framework::batch::createBatchFromSingleTransaction(
        std::shared_ptr<shared_model::interface::Transaction>(clone(tx))));
  }

  // 1. Create proposal and notify transaction processor about it
  auto proposal = std::make_shared<shared_model::proto::Proposal>(
      TestProposalBuilder().transactions(txs).build());

  prop_notifier.get_subscriber().on_next(proposal);
  prop_notifier.get_subscriber().on_completed();

  // empty transactions errors - all txs are valid
  verified_prop_notifier.get_subscriber().on_next(
      std::make_shared<iroha::validation::VerifiedProposalAndErrors>(
          std::make_pair(proposal, iroha::validation::TransactionsErrors{})));

  auto block = TestBlockBuilder().transactions(txs).build();

  // 2. Create block and notify transaction processor about it
  rxcpp::subjects::subject<std::shared_ptr<shared_model::interface::Block>>
      blocks_notifier;

  commit_notifier.get_subscriber().on_next(SynchronizationEvent{
      blocks_notifier.get_observable(), SynchronizationOutcomeType::kCommit});

  blocks_notifier.get_subscriber().on_next(
      std::shared_ptr<shared_model::interface::Block>(clone(block)));
  // Note blocks_notifier hasn't invoked on_completed, so
  // transactions are not commited

  SCOPED_TRACE("Stateful valid status verification");
  validateStatuses<shared_model::interface::StatefulValidTxResponse>(txs);
}

/**
 * @given transaction processor
 * @when transactions compose proposal which is sent to peer
 * communication service @and all transactions composed the block @and were
 * committed
 * @then for every transaction COMMIT status is returned
 */
TEST_F(TransactionProcessorTest, TransactionProcessorOnCommitTest) {
  std::vector<shared_model::proto::Transaction> txs;
  for (size_t i = 0; i < proposal_size; i++) {
    auto &&tx = TestTransactionBuilder().createdTime(i).build();
    txs.push_back(tx);
    status_map[tx.hash()] =
        status_builder.notReceived().txHash(tx.hash()).build();
  }

  EXPECT_CALL(*status_bus, publish(_))
      .Times(txs.size() * 3)
      .WillRepeatedly(testing::Invoke([this](auto response) {
        status_map[response->transactionHash()] = response;
      }));

  EXPECT_CALL(*mp, propagateBatchImpl(_)).Times(0);
  EXPECT_CALL(*pcs, propagate_transaction(_)).Times(txs.size());

  for (const auto &tx : txs) {
    tp->batchHandle(framework::batch::createBatchFromSingleTransaction(
        std::shared_ptr<shared_model::interface::Transaction>(clone(tx))));
  }

  // 1. Create proposal and notify transaction processor about it
  auto proposal = std::make_shared<shared_model::proto::Proposal>(
      TestProposalBuilder().transactions(txs).build());

  prop_notifier.get_subscriber().on_next(proposal);
  prop_notifier.get_subscriber().on_completed();

  // empty transactions errors - all txs are valid
  verified_prop_notifier.get_subscriber().on_next(
      std::make_shared<iroha::validation::VerifiedProposalAndErrors>(
          std::make_pair(proposal, iroha::validation::TransactionsErrors{})));

  auto block = TestBlockBuilder().transactions(txs).build();

  // 2. Create block and notify transaction processor about it
  SynchronizationEvent commit_event{
      rxcpp::observable<>::just(
          std::shared_ptr<shared_model::interface::Block>(clone(block))),
      SynchronizationOutcomeType::kCommit};
  commit_notifier.get_subscriber().on_next(commit_event);

  SCOPED_TRACE("Committed status verification");
  validateStatuses<shared_model::interface::CommittedTxResponse>(txs);
}

/**
 * @given transaction processor
 * @when transactions compose proposal which is sent to peer
 * communication service @and some transactions became part of block, while some
 * were not committed, failing stateful validation
 * @then for every transaction from block COMMIT status is returned @and
 * for every transaction, which failed stateful validation,
 * STATEFUL_INVALID_STATUS status is returned
 */
TEST_F(TransactionProcessorTest, TransactionProcessorInvalidTxsTest) {
  std::vector<shared_model::proto::Transaction> block_txs;
  for (size_t i = 0; i < block_size; i++) {
    auto &&tx = TestTransactionBuilder().createdTime(i).build();
    block_txs.push_back(tx);
    status_map[tx.hash()] =
        status_builder.notReceived().txHash(tx.hash()).build();
  }

  std::vector<shared_model::proto::Transaction> invalid_txs;

  for (size_t i = block_size; i < proposal_size; i++) {
    auto &&tx = TestTransactionBuilder().createdTime(i).build();
    invalid_txs.push_back(tx);
    status_map[tx.hash()] =
        status_builder.notReceived().txHash(tx.hash()).build();
  }

  // For all transactions from proposal
  // transaction will be published twice
  // (first that they are stateless
  // valid and second that they either
  // passed or not stateful validation)
  // Plus all transactions from block will
  // be committed and corresponding status will be sent
  EXPECT_CALL(*status_bus, publish(_))
      .Times(proposal_size * 2 + block_size)
      .WillRepeatedly(testing::Invoke([this](auto response) {
        status_map[response->transactionHash()] = response;
      }));

  auto proposal = std::make_shared<shared_model::proto::Proposal>(
      TestProposalBuilder()
          .transactions(boost::join(block_txs, invalid_txs))
          .build());

  prop_notifier.get_subscriber().on_next(proposal);
  prop_notifier.get_subscriber().on_completed();

  // trigger the verified event with txs, which we want to fail, as errors
  auto verified_proposal = std::make_shared<shared_model::proto::Proposal>(
      TestProposalBuilder().transactions(block_txs).build());
  auto txs_errors = iroha::validation::TransactionsErrors{};
  for (size_t i = 0; i < invalid_txs.size(); ++i) {
    txs_errors.push_back(std::make_pair(
        iroha::validation::CommandError{
            "SomeCommandName", "SomeCommandError", true, i},
        invalid_txs[i].hash()));
  }
  verified_prop_notifier.get_subscriber().on_next(
      std::make_shared<iroha::validation::VerifiedProposalAndErrors>(
          std::make_pair(verified_proposal, txs_errors)));

  auto block = TestBlockBuilder().transactions(block_txs).build();

  SynchronizationEvent commit_event{
      rxcpp::observable<>::just(
          std::shared_ptr<shared_model::interface::Block>(clone(block))),
      SynchronizationOutcomeType::kCommit};
  commit_notifier.get_subscriber().on_next(commit_event);

  {
    SCOPED_TRACE("Stateful invalid status verification");
    // check that all invalid transactions will have stateful invalid status
    validateStatuses<shared_model::interface::StatefulFailedTxResponse>(
        invalid_txs);
  }
  {
    SCOPED_TRACE("Committed status verification");
    // check that all transactions from block will be committed
    validateStatuses<shared_model::interface::CommittedTxResponse>(block_txs);
  }
}

/**
 * @given valid multisig tx
 * @when transaction_processor handle it
 * @then it goes to mst and after signing goes to PeerCommunicationService
 */
TEST_F(TransactionProcessorTest, MultisigTransaction) {
  std::shared_ptr<shared_model::interface::TransactionBatch> after_mst;
  auto mst_propagate =
      [&after_mst](
          std::shared_ptr<shared_model::interface::TransactionBatch> batch) {
        auto keypair1 =
            shared_model::crypto::DefaultCryptoAlgorithmType::generateKeypair();
        auto signedBlob1 = shared_model::crypto::CryptoSigner<>::sign(
            shared_model::crypto::Blob(batch->transactions().at(0)->payload()),
            keypair1);
        after_mst->addSignature(0, signedBlob1, keypair1.publicKey());
        auto keypair2 =
            shared_model::crypto::DefaultCryptoAlgorithmType::generateKeypair();
        auto signedBlob2 = shared_model::crypto::CryptoSigner<>::sign(
            shared_model::crypto::Blob(batch->transactions().at(0)->payload()),
            keypair2);
        after_mst->addSignature(0, signedBlob2, keypair2.publicKey());
      };
  EXPECT_CALL(*mp, propagateBatchImpl(_))
      .WillOnce(testing::Invoke(mst_propagate));
  EXPECT_CALL(*pcs, propagate_transaction(_)).Times(1);

  std::shared_ptr<shared_model::interface::Transaction> tx =
      clone(base_tx()
                .quorum(2)
                .build()
                .signAndAddSignature(
                    shared_model::crypto::DefaultCryptoAlgorithmType::
                        generateKeypair())
                .finish());

  tp->batchHandle(framework::batch::createBatchFromSingleTransaction(tx));
  mst_prepared_notifier.get_subscriber().on_next(after_mst);
}

/**
 * @given valid multisig tx
 * @when transaction_processor handle it
 * @then ensure after expiring it leads to MST_EXPIRED status
 */
TEST_F(TransactionProcessorTest, MultisigExpired) {
  EXPECT_CALL(*mp, propagateBatchImpl(_)).Times(1);
  EXPECT_CALL(*pcs, propagate_transaction(_)).Times(0);

  std::shared_ptr<shared_model::interface::Transaction> tx =
      clone(base_tx()
                .quorum(2)
                .build()
                .signAndAddSignature(
                    shared_model::crypto::DefaultCryptoAlgorithmType::
                        generateKeypair())
                .finish());
  EXPECT_CALL(*status_bus, publish(_))
      .WillOnce(testing::Invoke([](auto response) {
        ASSERT_NO_THROW(boost::apply_visitor(
            framework::SpecifiedVisitor<
                shared_model::interface::MstExpiredResponse>(),
            response->get()));
      }));
  tp->batchHandle(framework::batch::createBatchFromSingleTransaction(tx));
  mst_expired_notifier.get_subscriber().on_next(
      std::make_shared<shared_model::interface::TransactionBatch>(
          framework::batch::createBatchFromSingleTransaction(tx)));
}
