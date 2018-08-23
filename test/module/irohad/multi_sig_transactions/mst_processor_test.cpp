/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
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
  bool operator()(const DataType &batch) const override {
    return std::all_of(batch->transactions().begin(),
                       batch->transactions().end(),
                       [](const auto &tx) {
                         return boost::size(tx->signatures()) >= tx->quorum();
                       });
  }

  bool operator()(const DataType &batch, const TimeType &time) const override {
    return std::any_of(
        batch->transactions().begin(),
        batch->transactions().end(),
        [&time](const auto &tx) { return tx->createdTime() < time; });
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

  const shared_model::interface::types::CounterType time_now =
      iroha::time::now();
  const shared_model::interface::types::CounterType time_before = time_now - 1;
  const shared_model::interface::types::CounterType time_after = time_now + 1;

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
      make_test_subscriber<CallExact>(mst_processor->onPreparedBatches(), b),
      make_test_subscriber<CallExact>(mst_processor->onExpiredBatches(), c));
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
 * @when an incomplete batch is inserted
 *
 * @then check that:
 * state is not updated
 * AND absent prepared transactions
 * AND absent expired transactions
 */
TEST_F(MstProcessorTest, notCompletedTransactionUsecase) {
  // ---------------------------------| given |---------------------------------
  auto observers = initObservers(mst_processor, 0, 0, 0);

  // ---------------------------------| when |----------------------------------
  mst_processor->propagateBatch(
      addSignaturesFromKeyPairs(makeTestBatch(txBuilder(1)), 0, makeKey()));

  // ---------------------------------| then |----------------------------------
  check(observers);
}

/**
 * @given initialised mst processor
 * AND wrappers on mst observables
 *
 * @when the same transaction arrives with different signatures
 * AND the resulting set of signatures satisfies the account quorum number
 *
 * @then check that:
 * state is not updated
 * AND 1 prepared transaction
 * AND absent expired transactions
 */
TEST_F(MstProcessorTest, completedTransactionUsecase) {
  // ---------------------------------| given |---------------------------------
  auto observers = initObservers(mst_processor, 0, 1, 0);

  // ---------------------------------| when |----------------------------------
  mst_processor->propagateBatch(addSignaturesFromKeyPairs(
      makeTestBatch(txBuilder(1, time_now, 3)), 0, makeKey()));
  mst_processor->propagateBatch(addSignaturesFromKeyPairs(
      makeTestBatch(txBuilder(1, time_now, 3)), 0, makeKey()));
  mst_processor->propagateBatch(addSignaturesFromKeyPairs(
      makeTestBatch(txBuilder(1, time_now, 3)), 0, makeKey()));

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
  auto quorum = 1u;
  mst_processor->propagateBatch(addSignaturesFromKeyPairs(
      makeTestBatch(txBuilder(1, time_before, quorum)), 0, makeKey()));

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
  mst_processor->propagateBatch(addSignaturesFromKeyPairs(
      makeTestBatch(txBuilder(1, time_now, quorum)), 0, makeKey()));

  // ---------------------------------| when |----------------------------------
  auto another_peer = makePeer("another", "another_pubkey");
  auto transported_state = MstState::empty(std::make_shared<TestCompleter>());
  transported_state += addSignaturesFromKeyPairs(
      makeTestBatch(txBuilder(1, time_now, quorum)), 0, makeKey());
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
  auto quorum = 2u;
  mst_processor->propagateBatch(addSignaturesFromKeyPairs(
      makeTestBatch(txBuilder(1, time_after, quorum)), 0, makeKey()));
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
  another_peer_state += makeTestBatch(txBuilder(1));

  storage->apply(another_peer, another_peer_state);
  ASSERT_TRUE(storage->getDiffState(another_peer, time_now).isEmpty());

  // ---------------------------------| when |----------------------------------
  std::vector<std::shared_ptr<shared_model::interface::Peer>> peers{
      another_peer};
  propagation_subject.get_subscriber().on_next(peers);
}
