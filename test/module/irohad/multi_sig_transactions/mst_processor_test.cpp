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

#include <gtest/gtest.h>
#include "framework/test_subscriber.hpp"
#include "logger/logger.hpp"
#include "module/irohad/multi_sig_transactions/mst_mocks.hpp"
#include "module/irohad/multi_sig_transactions/mst_test_helpers.hpp"
#include "multi_sig_transactions/mst_processor_impl.hpp"
#include "multi_sig_transactions/storage/mst_storage_impl.hpp"

auto log_ = logger::log("MstProcessorTest");

using namespace std;
using namespace iroha;
using namespace iroha::model;
using namespace framework::test_subscriber;

using testing::Return;
using testing::_;

class TestCompleter : public Completer {
  bool operator()(const DataType transaction) const override {
    return transaction->signatures.size() >= transaction->quorum;
  }

  bool operator()(const DataType &tx, const TimeType &time) const override {
    return tx->created_ts < time;
  }
};

class MstProcessorTest : public testing::Test {
 public:
  // --------------------------------| fields |---------------------------------

  Peer own_peer = makePeer("iam", "1");
  /// propagation subject, useful for propagation control
  rxcpp::subjects::subject<PropagationStrategy::PropagationData>
      propagation_subject;
  /// use effective implementation of storage
  shared_ptr<MstStorage> storage;
  shared_ptr<FairMstProcessor> mst_processor;

  // ---------------------------------| mocks |---------------------------------

  shared_ptr<MockMstTransport> transport;
  shared_ptr<MockPropagationStrategy> propagation_strategy;
  shared_ptr<MockTimeProvider> time_provider;

 protected:
  void SetUp() override {
    transport = make_shared<MockMstTransport>();
    storage = make_shared<MstStorageStateImpl>(own_peer,
                                               make_shared<TestCompleter>());

    propagation_strategy = make_shared<MockPropagationStrategy>();
    EXPECT_CALL(*propagation_strategy, emitter())
        .WillOnce(Return(propagation_subject.get_observable()));

    time_provider = make_shared<MockTimeProvider>();
    EXPECT_CALL(*time_provider, getCurrentTime()).WillRepeatedly(Return(111));

    mst_processor = make_shared<FairMstProcessor>(
        transport, storage, propagation_strategy, time_provider);
  }
};

/**
 * @given initialised mst processor
 * AND wrappers on mst observables
 *
 * @when insert not-completed transaction
 *
 * @then check that:
 * state not updated
 * AND absent prepared transactions
 * AND absent expired transactions
 */
TEST_F(MstProcessorTest, notCompletedTransactionUsecase) {
  // ---------------------------------| given |---------------------------------
  auto onStateUpdateWrapper =
      make_test_subscriber<CallExact>(mst_processor->onStateUpdate(), 0);
  onStateUpdateWrapper.subscribe();

  auto onPreparedTransactionsWrapper = make_test_subscriber<CallExact>(
      mst_processor->onPreparedTransactions(), 0);
  onPreparedTransactionsWrapper.subscribe();

  auto onExpiredTransactionsWrapper = make_test_subscriber<CallExact>(
      mst_processor->onExpiredTransactions(), 0);
  onExpiredTransactionsWrapper.subscribe();

  // ---------------------------------| when |----------------------------------
  auto quorum = 2;
  auto creation_time = 112;
  mst_processor->propagateTransaction(
      makeTx("hash", "sign", quorum, creation_time));

  // ---------------------------------| then |----------------------------------
  ASSERT_TRUE(onStateUpdateWrapper.validate());
  ASSERT_TRUE(onPreparedTransactionsWrapper.validate());
  ASSERT_TRUE(onExpiredTransactionsWrapper.validate());
}

/**
 * @given initialised mst processor
 * AND wrappers on mst observables
 *
 * @when insert transactions that provide completed transaction
 *
 * @then check that:
 * state not updated
 * AND 1 prepared transaction
 * AND absent expired transactions
 */
TEST_F(MstProcessorTest, completedTransactionUsecase) {
  // ---------------------------------| given |---------------------------------
  auto onStateUpdateWrapper =
      make_test_subscriber<CallExact>(mst_processor->onStateUpdate(), 0);
  onStateUpdateWrapper.subscribe();

  auto onPreparedTransactionsWrapper = make_test_subscriber<CallExact>(
      mst_processor->onPreparedTransactions(), 1);
  onPreparedTransactionsWrapper.subscribe();

  auto onExpiredTransactionsWrapper = make_test_subscriber<CallExact>(
      mst_processor->onExpiredTransactions(), 0);
  onExpiredTransactionsWrapper.subscribe();

  // ---------------------------------| when |----------------------------------
  auto quorum = 2;
  auto creation_time = 112;
  mst_processor->propagateTransaction(
      makeTx("hash", "sign_one", quorum, creation_time));
  mst_processor->propagateTransaction(
      makeTx("hash", "sign_two", quorum, creation_time));

  // ---------------------------------| then |----------------------------------
  ASSERT_TRUE(onStateUpdateWrapper.validate());
  ASSERT_TRUE(onPreparedTransactionsWrapper.validate());
  ASSERT_TRUE(onExpiredTransactionsWrapper.validate());
}

/**
 * @given initialised mst processor
 * AND wrappers on mst observables
 *
 * @when insert (by propagate_transaction) method transaction that already
 * expired with quorum one
 *
 * @then check that:
 * state not updated
 * AND 0 prepared transaction (although quorum 1)
 * AND 1 expired transactions
 */
TEST_F(MstProcessorTest, expiredTransactionUsecase) {
  // ---------------------------------| given |---------------------------------
  auto onStateUpdateWrapper =
      make_test_subscriber<CallExact>(mst_processor->onStateUpdate(), 0);
  onStateUpdateWrapper.subscribe();

  auto onPreparedTransactionsWrapper = make_test_subscriber<CallExact>(
      mst_processor->onPreparedTransactions(), 0);
  onPreparedTransactionsWrapper.subscribe();

  auto onExpiredTransactionsWrapper = make_test_subscriber<CallExact>(
      mst_processor->onExpiredTransactions(), 1);
  onExpiredTransactionsWrapper.subscribe();

  // ---------------------------------| when |----------------------------------
  auto quorum = 1;
  auto timeBefore = 110;
  mst_processor->propagateTransaction(
      makeTx("another hash", "sign", quorum, timeBefore));

  // ---------------------------------| then |----------------------------------
  ASSERT_TRUE(onStateUpdateWrapper.validate());
  ASSERT_TRUE(onPreparedTransactionsWrapper.validate());
  ASSERT_TRUE(onExpiredTransactionsWrapper.validate());
}

/**
 * @given initialised mst processor
 * AND our state contains one transactions TX with quorum 2
 * AND wrappers on mst observables
 *
 * @when received new state from other peer via transport,
 * that contains TX with another signature
 *
 * @then check that:
 * state updated
 * AND 1 prepared transaction (although quorum 1)
 * AND 0 expired transactions
 */
TEST_F(MstProcessorTest, onUpdateFromTransportUsecase) {
  // ---------------------------------| given |---------------------------------
  auto onStateUpdateWrapper =
      make_test_subscriber<CallExact>(mst_processor->onStateUpdate(), 1);
  onStateUpdateWrapper.subscribe();

  auto onPreparedTransactionsWrapper = make_test_subscriber<CallExact>(
      mst_processor->onPreparedTransactions(), 1);
  onPreparedTransactionsWrapper.subscribe();

  auto onExpiredTransactionsWrapper = make_test_subscriber<CallExact>(
      mst_processor->onExpiredTransactions(), 0);
  onExpiredTransactionsWrapper.subscribe();

  auto quorum = 2;
  auto timeAfter = 112;
  mst_processor->propagateTransaction(
      makeTx("hash", "sign_one", quorum, timeAfter));

  // ---------------------------------| when |----------------------------------
  auto another_peer = makePeer("another", "another_pubkey");
  auto transported_state = MstState::empty(make_shared<TestCompleter>());
  transported_state += makeTx("hash", "sign_two", quorum, timeAfter);
  mst_processor->onNewState(another_peer, transported_state);

  // ---------------------------------| then |----------------------------------
  ASSERT_TRUE(onStateUpdateWrapper.validate());
  ASSERT_TRUE(onPreparedTransactionsWrapper.validate());
  ASSERT_TRUE(onExpiredTransactionsWrapper.validate());
}

/**
 * @given initialised mst processor
 * AND our state contains one transaction
 *
 * @when received notification about new propagation
 *
 * @then check that:
 * transport invoked for all peers
 */

TEST_F(MstProcessorTest, onNewPropagationUsecase) {
  // ---------------------------------| given |---------------------------------
  auto quorum = 2;
  auto timeAfter = 112;
  mst_processor->propagateTransaction(
      makeTx("hash", "sign_one", quorum, timeAfter));
  EXPECT_CALL(*transport, sendState(_, _)).Times(2);

  // ---------------------------------| when |----------------------------------
  std::vector<Peer> peers = {makePeer("one", "sign_one"),
                             makePeer("two", "sign_two")};
  propagation_subject.get_subscriber().on_next(peers);
}
