/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <boost/range/join.hpp>
#include "builders/protobuf/common_objects/proto_signature_builder.hpp"
#include "builders/protobuf/proposal.hpp"
#include "builders/protobuf/transaction.hpp"
#include "framework/test_subscriber.hpp"
#include "interfaces/utils/specified_visitor.hpp"
#include "module/irohad/multi_sig_transactions/mst_mocks.hpp"
#include "module/irohad/network/network_mocks.hpp"
#include "module/shared_model/builders/protobuf/test_block_builder.hpp"
#include "module/shared_model/builders/protobuf/test_proposal_builder.hpp"
#include "module/shared_model/builders/protobuf/test_transaction_builder.hpp"
#include "module/shared_model/builders/transaction_responses/transaction_builders_common.hpp"
#include "torii/processor/transaction_processor_impl.hpp"

using namespace iroha;
using namespace iroha::network;
using namespace iroha::torii;
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

    EXPECT_CALL(*mp, onPreparedTransactionsImpl())
        .WillRepeatedly(Return(mst_prepared_notifier.get_observable()));
    EXPECT_CALL(*mp, onExpiredTransactionsImpl())
        .WillRepeatedly(Return(mst_expired_notifier.get_observable()));

    tp = std::make_shared<TransactionProcessorImpl>(pcs, mp);
  }

  auto base_tx() {
    return shared_model::proto::TransactionBuilder()
        .creatorAccountId("user@domain")
        .createdTime(iroha::time::now())
        .setAccountQuorum("user@domain", 2)
        .quorum(1);
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
      boost::apply_visitor(verifyType<Status>(), tx_status->second->get());
    }
  }

  rxcpp::subjects::subject<iroha::DataType> mst_prepared_notifier;
  rxcpp::subjects::subject<iroha::DataType> mst_expired_notifier;

  std::shared_ptr<MockPeerCommunicationService> pcs;
  std::shared_ptr<TransactionProcessorImpl> tp;
  std::shared_ptr<MockMstProcessor> mp;

  StatusMapType status_map;
  shared_model::builder::TransactionStatusBuilder<
      shared_model::proto::TransactionStatusBuilder>
      status_builder;

  rxcpp::subjects::subject<std::shared_ptr<shared_model::interface::Proposal>>
      prop_notifier;
  rxcpp::subjects::subject<Commit> commit_notifier;

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
    auto &&tx = TestTransactionBuilder().createdTime(i).build();
    txs.push_back(tx);
    status_map[tx.hash()] =
        status_builder.notReceived().txHash(tx.hash()).build();
  }

  auto wrapper =
      make_test_subscriber<CallExact>(tp->transactionNotifier(), proposal_size);
  wrapper.subscribe([this](auto response) {
    status_map[response->transactionHash()] = response;
  });

  EXPECT_CALL(*mp, propagateTransactionImpl(_)).Times(0);
  EXPECT_CALL(*pcs, propagate_transaction(_)).Times(txs.size());

  for (const auto &tx : txs) {
    tp->transactionHandle(
        std::shared_ptr<shared_model::interface::Transaction>(clone(tx)));
  }

  // create proposal and notify about it
  auto proposal = std::make_shared<shared_model::proto::Proposal>(
      TestProposalBuilder().transactions(txs).build());

  prop_notifier.get_subscriber().on_next(proposal);
  prop_notifier.get_subscriber().on_completed();

  ASSERT_TRUE(wrapper.validate());

  SCOPED_TRACE("Stateless valid status verification");
  validateStatuses<shared_model::interface::StatelessValidTxResponse>(txs);
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

  auto wrapper = make_test_subscriber<CallExact>(
      tp->transactionNotifier(),
      txs.size() * 2);  // every transaction is notified that it is stateless
                        // valid and  then stateful valid
  wrapper.subscribe([this](auto response) {
    status_map[response->transactionHash()] = response;
  });

  EXPECT_CALL(*mp, propagateTransactionImpl(_)).Times(0);
  EXPECT_CALL(*pcs, propagate_transaction(_)).Times(txs.size());

  for (const auto &tx : txs) {
    tp->transactionHandle(
        std::shared_ptr<shared_model::interface::Transaction>(clone(tx)));
  }

  // 1. Create proposal and notify transaction processor about it
  auto proposal = std::make_shared<shared_model::proto::Proposal>(
      TestProposalBuilder().transactions(txs).build());

  prop_notifier.get_subscriber().on_next(proposal);
  prop_notifier.get_subscriber().on_completed();

  auto block = TestBlockBuilder().transactions(txs).build();

  // 2. Create block and notify transaction processor about it
  rxcpp::subjects::subject<std::shared_ptr<shared_model::interface::Block>>
      blocks_notifier;

  commit_notifier.get_subscriber().on_next(blocks_notifier.get_observable());

  blocks_notifier.get_subscriber().on_next(
      std::shared_ptr<shared_model::interface::Block>(clone(block)));
  // Note blocks_notifier hasn't invoked on_completed, so
  // transactions are not commited

  ASSERT_TRUE(wrapper.validate());

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

  auto wrapper = make_test_subscriber<CallExact>(
      tp->transactionNotifier(),
      txs.size() * 3);  // evey transaction is notified that it is first
                        // stateless valid, then stateful valid and
                        // eventually committed
  wrapper.subscribe([this](auto response) {
    status_map[response->transactionHash()] = response;
  });

  EXPECT_CALL(*mp, propagateTransactionImpl(_)).Times(0);
  EXPECT_CALL(*pcs, propagate_transaction(_)).Times(txs.size());

  for (const auto &tx : txs) {
    tp->transactionHandle(
        std::shared_ptr<shared_model::interface::Transaction>(clone(tx)));
  }

  // 1. Create proposal and notify transaction processor about it
  auto proposal = std::make_shared<shared_model::proto::Proposal>(
      TestProposalBuilder().transactions(txs).build());

  prop_notifier.get_subscriber().on_next(proposal);
  prop_notifier.get_subscriber().on_completed();

  auto block = TestBlockBuilder().transactions(txs).build();

  // 2. Create block and notify transaction processor about it
  Commit single_commit = rxcpp::observable<>::just(
      std::shared_ptr<shared_model::interface::Block>(clone(block)));
  commit_notifier.get_subscriber().on_next(single_commit);

  ASSERT_TRUE(wrapper.validate());

  SCOPED_TRACE("Committed status verification");
  validateStatuses<shared_model::interface::CommittedTxResponse>(txs);
}

/**
 * @given transaction processor
 * @when transactions compose proposal which is sent to peer
 * communication service @and some transactions became part of block, while some
 * were not committed
 * @then for every transaction from block COMMIT status is returned @and for
 * every transaction not from block STATEFUL_INVALID_STATUS was returned
 */
TEST_F(TransactionProcessorTest, TransactionProcessorInvalidTxsTest) {
  std::vector<shared_model::proto::Transaction> block_txs;
  for (size_t i = 0; i < block_size; i++) {
    auto &&tx = TestTransactionBuilder().createdTime(i).build();
    block_txs.push_back(tx);
    status_map[tx.hash()] =
        status_builder.notReceived().txHash(tx.hash()).build();
  }

  std::vector<shared_model::proto::Transaction>
      invalid_txs;  // transactions will be stateful invalid if appeared
                    // in proposal but didn't appear in block
  for (size_t i = block_size; i < proposal_size; i++) {
    auto &&tx = TestTransactionBuilder().createdTime(i).build();
    invalid_txs.push_back(tx);
    status_map[tx.hash()] =
        status_builder.notReceived().txHash(tx.hash()).build();
  }

  auto wrapper = make_test_subscriber<CallExact>(
      tp->transactionNotifier(),
      proposal_size * 2
          + block_size);  // For all transactions from proposal
                          // transaction notifier will notified
                          // twice (first that they are stateless
                          // valid and second that they either
                          // passed or not stateful validation)
                          // Plus all transactions from block will
                          // be committed and corresponding status will be sent

  wrapper.subscribe([this](auto response) {
    status_map[response->transactionHash()] = response;
  });

  auto proposal = std::make_shared<shared_model::proto::Proposal>(
      TestProposalBuilder()
          .transactions(boost::join(block_txs, invalid_txs))
          .build());

  prop_notifier.get_subscriber().on_next(proposal);
  prop_notifier.get_subscriber().on_completed();

  auto block = TestBlockBuilder().transactions(block_txs).build();

  Commit single_commit = rxcpp::observable<>::just(
      std::shared_ptr<shared_model::interface::Block>(clone(block)));
  commit_notifier.get_subscriber().on_next(single_commit);
  ASSERT_TRUE(wrapper.validate());

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
  std::shared_ptr<shared_model::proto::Transaction> after_mst;
  auto mst_propagate =
      [&after_mst](std::shared_ptr<shared_model::interface::Transaction> tx) {
        after_mst =
            std::static_pointer_cast<shared_model::proto::Transaction>(tx);
        auto keypair1 =
            shared_model::crypto::DefaultCryptoAlgorithmType::generateKeypair();
        auto signedBlob1 = shared_model::crypto::CryptoSigner<>::sign(
            shared_model::crypto::Blob(after_mst->payload()), keypair1);
        after_mst->addSignature(signedBlob1, keypair1.publicKey());
        auto keypair2 =
            shared_model::crypto::DefaultCryptoAlgorithmType::generateKeypair();
        auto signedBlob2 = shared_model::crypto::CryptoSigner<>::sign(
            shared_model::crypto::Blob(after_mst->payload()), keypair2);
        after_mst->addSignature(signedBlob2, keypair2.publicKey());
      };
  EXPECT_CALL(*mp, propagateTransactionImpl(_))
      .WillOnce(testing::Invoke(mst_propagate));
  EXPECT_CALL(*pcs, propagate_transaction(_)).Times(1);

  std::shared_ptr<shared_model::interface::Transaction> tx =
      clone(base_tx().quorum(2).build().signAndAddSignature(
          shared_model::crypto::DefaultCryptoAlgorithmType::generateKeypair()));

  tp->transactionHandle(tx);
  mst_prepared_notifier.get_subscriber().on_next(after_mst);
}

/**
 * @given valid multisig tx
 * @when transaction_processor handle it
 * @then ensure after expiring it leads to MST_EXPIRED status
 */
TEST_F(TransactionProcessorTest, MultisigExpired) {
  EXPECT_CALL(*mp, propagateTransactionImpl(_)).Times(1);
  EXPECT_CALL(*pcs, propagate_transaction(_)).Times(0);

  std::shared_ptr<shared_model::interface::Transaction> tx =
      clone(base_tx().quorum(2).build().signAndAddSignature(
          shared_model::crypto::DefaultCryptoAlgorithmType::generateKeypair()));

  auto wrapper = make_test_subscriber<CallExact>(tp->transactionNotifier(), 1);
  wrapper.subscribe([](auto response) {
    ASSERT_TRUE(
        boost::apply_visitor(shared_model::interface::SpecifiedVisitor<
                                 shared_model::interface::MstExpiredResponse>(),
                             response->get()));
  });
  tp->transactionHandle(tx);
  mst_expired_notifier.get_subscriber().on_next(tx);

  ASSERT_TRUE(wrapper.validate());
}
