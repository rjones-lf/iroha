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

#include "module/irohad/network/network_mocks.hpp"

#include "framework/test_subscriber.hpp"
#include "model/transaction_response.hpp"
#include "module/shared_model/builders/protobuf/test_block_builder.hpp"
#include "module/shared_model/builders/protobuf/test_proposal_builder.hpp"
#include "module/shared_model/builders/protobuf/test_transaction_builder.hpp"
#include "torii/processor/transaction_processor_impl.hpp"

using namespace iroha;
using namespace iroha::network;
using namespace iroha::torii;
using namespace iroha::model;
using namespace framework::test_subscriber;

using ::testing::_;
using ::testing::Return;
using ::testing::Invoke;

class TransactionProcessorTest : public ::testing::Test {
 public:
  void SetUp() override {
    pcs = std::make_shared<MockPeerCommunicationService>();

    EXPECT_CALL(*pcs, on_proposal())
        .WillOnce(Return(prop_notifier.get_observable()));

    EXPECT_CALL(*pcs, on_commit())
        .WillRepeatedly(Return(commit_notifier.get_observable()));

    tp = std::make_shared<TransactionProcessorImpl>(pcs);
  }

  void TearDown() override {
    pcs.reset();
    tp.reset();
  }

 protected:
  using StatusMapType = std::unordered_map<
      shared_model::crypto::Hash,
      std::shared_ptr<shared_model::interface::TransactionResponse>,
      shared_model::crypto::Hash::Hasher>;

  /// Compare operator between shared_model transactions to allow set operations
  /// (difference, includes)
  struct TxCompare {
    bool operator()(const shared_model::interface::Transaction &lhs,
                    const shared_model::interface::Transaction &rhs) const {
      return lhs.hash().hex() < rhs.hash().hex();
    }
  };

  using TxSetType = std::set<const shared_model::proto::Transaction, TxCompare>;

  /**
   * Sends transactions to peer communication service and creates proposal from
   * them
   * @param txs Transactions sent to transaction processor
   */
  void proposalTest(TxSetType &txs) {
    /// Transactions to compose proposal
    std::vector<const shared_model::proto::Transaction> proposal_txs;

    auto size = txs.size();
    EXPECT_CALL(*pcs, propagate_transaction(_))
        .Times(txs.size())
        .WillRepeatedly(Invoke([this, &proposal_txs, &size](auto tx) {
          // saturate proposal_txs vector until it has all transactions from txs
          // set
          proposal_txs.push_back(
              *std::static_pointer_cast<const shared_model::proto::Transaction>(
                  tx));

          // create proposal when proposal_txs has all transactions from txs and
          // notify transaction processor about it
          if (proposal_txs.size() == size) {
            auto proposal = std::make_shared<shared_model::proto::Proposal>(
                TestProposalBuilder().transactions(proposal_txs).build());

            prop_notifier.get_subscriber().on_next(proposal);
            prop_notifier.get_subscriber().on_completed();
          }
        }));

    for (const auto &tx : txs) {
      tp->transactionHandle(
          std::shared_ptr<shared_model::interface::Transaction>(tx.copy()));
    }
  }

  /**
   * Sends transactions to peer communication service and creates proposal from
   * them
   * @param txs transactions sent to peer communication service
   */
  void blockTest(TxSetType &txs) {
    /// Transactions to compose block
    std::vector<const shared_model::proto::Transaction> block_txs;

    auto size = txs.size();
    EXPECT_CALL(*pcs, propagate_transaction(_))
        .Times(txs.size())
        .WillRepeatedly(Invoke(
            [this, &block_txs, &size](
                std::shared_ptr<const shared_model::interface::Transaction>
                    tx) {
              // saturate block_txs vector until it has all transactions from
              // txs set
              block_txs.push_back(*std::static_pointer_cast<
                                  const shared_model::proto::Transaction>(tx));
              if (block_txs.size() == size) {
                // 1. Create proposal and notify transaction processor about it
                auto proposal = std::make_shared<shared_model::proto::Proposal>(
                    TestProposalBuilder().transactions(block_txs).build());

                prop_notifier.get_subscriber().on_next(proposal);
                prop_notifier.get_subscriber().on_completed();

                auto block = TestBlockBuilder()
                                 .height(1)
                                 .txNumber(size)
                                 .createdTime(iroha::time::now())
                                 .transactions(block_txs)
                                 .prevHash(shared_model::crypto::Hash(
                                     std::string(32, '0')))
                                 .build();

                // 2. Create block and notify transaction processor about it
                rxcpp::subjects::subject<
                    std::shared_ptr<shared_model::interface::Block>>
                    blocks_notifier;

                commit_notifier.get_subscriber().on_next(
                    blocks_notifier.get_observable());

                blocks_notifier.get_subscriber().on_next(
                    std::shared_ptr<shared_model::interface::Block>(
                        block.copy()));
                // Note blocks_notifier hasn't invoked on_completed, so
                // transactions are not commited
              }
            }));

    for (const auto &tx : txs) {
      tp->transactionHandle(
          std::shared_ptr<shared_model::interface::Transaction>(tx.copy()));
    }
  }

  /**
   * Sends transactions to peer communication service
   * @param proposal_txs transactions used to compose proposal
   * @param block_txs transactions used to compose block. Should be subset of
   * proposal_txs.
   */
  void commitTest(TxSetType &proposal_txs, TxSetType &block_txs) {
    // check if block_txs is subset of proposal_txs
    ASSERT_TRUE(std::includes(proposal_txs.begin(),
                              proposal_txs.end(),
                              block_txs.begin(),
                              block_txs.end(),
                              TxCompare()));

    /// transactions used to compose proposal (substitutes ordering service)
    std::vector<const shared_model::proto::Transaction> prop_txs_vector;
    /// transactions used to compose block (substitutes block creator)
    std::vector<const shared_model::proto::Transaction> block_txs_vector;

    EXPECT_CALL(*pcs, propagate_transaction(_))
        .Times(proposal_txs.size())
        .WillRepeatedly(Invoke([this,
                                &prop_txs_vector,
                                &block_txs_vector,
                                &proposal_txs,
                                &block_txs](auto tx) {
          // cast to shared_model::proto::Transaction
          auto proto_tx =
              *std::static_pointer_cast<const shared_model::proto::Transaction>(
                  tx);

          // check if transaction should belong to proposal and add it to
          // proposal transactions
          if (proposal_txs.find(proto_tx) != proposal_txs.end()) {
            prop_txs_vector.push_back(proto_tx);
          }
          // check if transaction should belong to block and add it to block
          // transactions
          if (block_txs.find(proto_tx) != block_txs.end()) {
            block_txs_vector.push_back(proto_tx);
          }

          // if proposal tx list and block tx list if ready, compose proposal
          // and block
          if (block_txs_vector.size() == block_txs.size()
              and prop_txs_vector.size() == proposal_txs.size()) {
            auto proposal = std::make_shared<shared_model::proto::Proposal>(
                TestProposalBuilder().transactions(prop_txs_vector).build());

            prop_notifier.get_subscriber().on_next(proposal);
            prop_notifier.get_subscriber().on_completed();

            auto block =
                TestBlockBuilder()
                    .height(1)
                    .txNumber(block_txs_vector.size())
                    .createdTime(iroha::time::now())
                    .transactions(block_txs_vector)
                    .prevHash(shared_model::crypto::Hash(std::string(32, '0')))
                    .build();

            Commit single_commit = rxcpp::observable<>::just(
                std::shared_ptr<shared_model::interface::Block>(block.copy()));
            commit_notifier.get_subscriber().on_next(single_commit);
          }
        }));

    /// send all transactions to transaction processor
    for (const auto &tx : proposal_txs) {
      tp->transactionHandle(
          std::shared_ptr<shared_model::interface::Transaction>(tx.copy()));
    }
  }

  /**
   * Checks if all transactions have corresponding status
   * @param transactions transactions to check status
   * @param status to be checked
   */
  template <typename Status>
  void validateStatuses(const TxSetType &transactions) {
    for (const auto &tx : transactions) {
      auto tx_status = status_map.find(tx.hash());
      ASSERT_NE(tx_status, status_map.end());
      boost::apply_visitor(
          [this](auto val) {
            if (std::is_same<decltype(val), Status>::value) {
              SUCCEED();
            } else {
              FAIL() << "obtained: " << typeid(decltype(val)).name()
                     << ", expected: " << typeid(Status).name() << std::endl;
            }
          },
          tx_status->second->get());
    }
  }

  std::shared_ptr<MockPeerCommunicationService> pcs;
  std::shared_ptr<TransactionProcessorImpl> tp;

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
  TxSetType txs;
  size_t size = 5;
  for (size_t i = 0; i < size; i++) {
    auto tx = TestTransactionBuilder().txCounter(i).build();
    txs.insert(tx);
    status_map[tx.hash()] =
        status_builder.notReceived().txHash(tx.hash()).build();
  }

  auto wrapper =
      make_test_subscriber<CallExact>(tp->transactionNotifier(), size);
  wrapper.subscribe([this](auto response) {
    //    auto resp = static_cast<TransactionResponse &>(*response);
    //    auto hash = shared_model::crypto::Hash(resp.tx_hash);
    status_map[response->transactionHash()] = response;
  });

  proposalTest(txs);
  ASSERT_TRUE(wrapper.validate());

  validateStatuses<shared_model::detail::PolymorphicWrapper<
      shared_model::interface::StatelessValidTxResponse>>(txs);
}

/**
 * @given transaction processor
 * @when transactions compose proposal which is sent to peer
 * communication service @and all transactions composed the block
 * @then for every transaction STATEFUL_VALID status is returned
 */
TEST_F(TransactionProcessorTest, TransactionProcessorBlockCreatedTest) {
  TxSetType txs;
  for (size_t i = 0; i < proposal_size; i++) {
    auto tx = TestTransactionBuilder().txCounter(i).build();
    txs.insert(tx);
    status_map[tx.hash()] =
        status_builder.notReceived().txHash(tx.hash()).build();
  }

  auto wrapper = make_test_subscriber<CallExact>(
      tp->transactionNotifier(), proposal_size * 2);  // every transaction is
                                                      // notified that it is
                                                      // stateless valid and
                                                      // then stateful valid
  wrapper.subscribe([this](auto response) {
    status_map[response->transactionHash()] = response;
  });

  blockTest(txs);
  ASSERT_TRUE(wrapper.validate());

  validateStatuses<shared_model::detail::PolymorphicWrapper<
      shared_model::interface::StatefulValidTxResponse>>(txs);
}

/**
 * @given transaction processor
 * @when transactions compose proposal which is sent to peer
 * communication service @and all transactions composed the block @and were
 * committed
 * @then for every transaction COMMIT status is returned
 */
TEST_F(TransactionProcessorTest, TransactionProcessorOnCommitTest) {
  TxSetType txs;
  for (size_t i = 0; i < proposal_size; i++) {
    auto tx = TestTransactionBuilder().txCounter(i).build();
    txs.insert(tx);
    status_map[tx.hash()] =
        status_builder.notReceived().txHash(tx.hash()).build();
  }

  auto wrapper = make_test_subscriber<CallExact>(
      tp->transactionNotifier(),
      proposal_size * 3);  // evey transaction is notified that it is first
                           // stateless valid, then stateful valid and
                           // eventually committed
  wrapper.subscribe([this](auto response) {
    status_map[response->transactionHash()] = response;
    std::cout << response->toString() << std::endl;
  });

  commitTest(txs, txs);
  ASSERT_TRUE(wrapper.validate());

  validateStatuses<shared_model::detail::PolymorphicWrapper<
      shared_model::interface::CommittedTxResponse>>(txs);
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
  TxSetType proposal_txs;
  for (size_t i = 0; i < proposal_size; i++) {
    auto tx = TestTransactionBuilder().txCounter(i).build();
    proposal_txs.insert(tx);
    status_map[tx.hash()] =
        status_builder.notReceived().txHash(tx.hash()).build();
  }

  TxSetType block_txs(proposal_txs.begin(),
                      std::next(proposal_txs.begin(), block_size));

  TxSetType invalid_txs;  // transactions will be stateful invalid if appeared
                          // in proposal but didn't appear in block
  std::set_difference(proposal_txs.begin(),
                      proposal_txs.end(),
                      block_txs.begin(),
                      block_txs.end(),
                      std::inserter(invalid_txs, invalid_txs.begin()),
                      TxCompare());

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

  commitTest(proposal_txs, block_txs);
  ASSERT_TRUE(wrapper.validate());

  // check that all invalid transactions will have stateful invalid status
  validateStatuses<shared_model::detail::PolymorphicWrapper<
      shared_model::interface::StatefulFailedTxResponse>>(invalid_txs);
  // check that all transactions from block will be committed
  validateStatuses<shared_model::detail::PolymorphicWrapper<
      shared_model::interface::CommittedTxResponse>>(block_txs);
}
