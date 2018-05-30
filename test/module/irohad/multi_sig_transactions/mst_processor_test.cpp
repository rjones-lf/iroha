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
#include <tuple>
#include "datetime/time.hpp"
#include "framework/test_subscriber.hpp"
#include "logger/logger.hpp"
#include "module/irohad/multi_sig_transactions/mst_mocks.hpp"
#include "module/irohad/multi_sig_transactions/mst_test_helpers.hpp"
#include "multi_sig_transactions/mst_processor_impl.hpp"
#include "multi_sig_transactions/storage/mst_storage_impl.hpp"

auto log_ = logger::log("MstProcessorTest");

using namespace iroha;
using namespace framework::test_subscriber;

using testing::_;
using testing::Return;

class TestCompleter : public Completer {
  bool operator()(const DataType transaction) const override {
    return boost::size(transaction->signatures()) >= transaction->quorum();
  }

  bool operator()(const DataType &tx, const TimeType &time) const override {
    return tx->createdTime() < time;
  }
};

class MstProcessorTest : public testing::Test {
 public:
  // --------------------------------| fields |---------------------------------

  /// propagation subject, useful for propagation control
  rxcpp::subjects::subject<PropagationStrategy::PropagationData>
      propagation_subject;
  /// use effective implementation of storage
  std::shared_ptr<MstStorage> storage;
  std::shared_ptr<FairMstProcessor> mst_processor;

  // ---------------------------------| mocks |---------------------------------

  std::shared_ptr<MockMstTransport> transport;
  std::shared_ptr<MockPropagationStrategy> propagation_strategy;
  std::shared_ptr<MockTimeProvider> time_provider;

  const shared_model::interface::types::CounterType time_before =
      iroha::time::now() - 10000;
  const shared_model::interface::types::CounterType time_now =
      iroha::time::now();
  const shared_model::interface::types::CounterType time_after =
      iroha::time::now() + 10000;

 protected:
  void SetUp() override {
    transport = std::make_shared<MockMstTransport>();
    storage = std::make_shared<MstStorageStateImpl>(
        std::make_shared<TestCompleter>());

    propagation_strategy = std::make_shared<MockPropagationStrategy>();
    EXPECT_CALL(*propagation_strategy, emitter())
        .WillOnce(Return(propagation_subject.get_observable()));

    time_provider = std::make_shared<MockTimeProvider>();
    EXPECT_CALL(*time_provider, getCurrentTime())
        .WillRepeatedly(Return(time_now));

    mst_processor = std::make_shared<FairMstProcessor>(
        transport, storage, propagation_strategy, time_provider);
  }
};

/*
 * Initialize observables of mst processor
 */
auto initObservers(std::shared_ptr<FairMstProcessor> mst_processor,
                   int a,
                   int b,
                   int c) {
  auto obs = std::make_tuple(
      make_test_subscriber<CallExact>(mst_processor->onStateUpdate(), a),
      make_test_subscriber<CallExact>(mst_processor->onPreparedTransactions(),
                                      b),
      make_test_subscriber<CallExact>(mst_processor->onExpiredTransactions(),
                                      c));
  std::get<0>(obs).subscribe();
  std::get<1>(obs).subscribe();
  std::get<2>(obs).subscribe();
  return obs;
}

/*
 * Make sure that observables in the valid state
 */
template <typename T>
void check(T &t) {
  ASSERT_TRUE(std::get<0>(t).validate());
  ASSERT_TRUE(std::get<1>(t).validate());
  ASSERT_TRUE(std::get<2>(t).validate());
}

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
  auto observers = initObservers(mst_processor, 0, 0, 0);

  // ---------------------------------| when |----------------------------------
  mst_processor->propagateTransaction(makeTx(1));

  // ---------------------------------| then |----------------------------------
  check(observers);
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
  auto observers = initObservers(mst_processor, 0, 1, 0);

  // ---------------------------------| when |----------------------------------
  auto time = iroha::time::now();
  mst_processor->propagateTransaction(makeTx(1, time));
  mst_processor->propagateTransaction(makeTx(1, time));
  mst_processor->propagateTransaction(makeTx(1, time));

  // ---------------------------------| then |----------------------------------
  check(observers);
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
  auto observers = initObservers(mst_processor, 0, 0, 1);

  // ---------------------------------| when |----------------------------------
  auto quorum = 1;
  mst_processor->propagateTransaction(
      makeTx(1, time_before, makeKey(), quorum));

  // ---------------------------------| then |----------------------------------
  check(observers);
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
  auto observers = initObservers(mst_processor, 1, 1, 0);

  auto quorum = 2;
  mst_processor->propagateTransaction(makeTx(1, time_after, makeKey(), quorum));

  // ---------------------------------| when |----------------------------------
  auto another_peer = makePeer("another", "another_pubkey");
  auto transported_state = MstState::empty(std::make_shared<TestCompleter>());
  transported_state += makeTx(1, time_after, makeKey(), quorum);
  mst_processor->onNewState(another_peer, transported_state);

  // ---------------------------------| then |----------------------------------
  check(observers);
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
  mst_processor->propagateTransaction(makeTx(1, time_after, makeKey(), quorum));
  EXPECT_CALL(*transport, sendState(_, _)).Times(2);

  // ---------------------------------| when |----------------------------------
  std::vector<std::shared_ptr<shared_model::interface::Peer>> peers{
      makePeer("one", "sign_one"), makePeer("two", "sign_two")};
  propagation_subject.get_subscriber().on_next(peers);
}

/**
 * @given initialized mst processor
 * AND our state contains one transaction
 * AND one peer with the same state as our
 *
 * @when received notification about new propagation
 *
 * @then check that transport was not invoked
 */
TEST_F(MstProcessorTest, emptyStatePropagation) {
  // ---------------------------------| then |----------------------------------
  EXPECT_CALL(*transport, sendState(_, _)).Times(0);

  // ---------------------------------| given |---------------------------------
  auto another_peer = makePeer("another", "another_pubkey");

  auto another_peer_state = MstState::empty();
  another_peer_state += makeTx(1);

  storage->apply(another_peer, another_peer_state);
  ASSERT_TRUE(storage->getDiffState(another_peer, time_now).isEmpty());

  // ---------------------------------| when |----------------------------------
  std::vector<std::shared_ptr<shared_model::interface::Peer>> peers{
      another_peer};
  propagation_subject.get_subscriber().on_next(peers);
}
