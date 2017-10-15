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
#include "logger/logger.hpp"
#include "module/irohad/multi_sig_transactions/mst_mocks.hpp"
#include "module/irohad/multi_sig_transactions/mst_test_helpers.hpp"
#include "multi_sig_transactions/mst_processor_impl.hpp"
#include "multi_sig_transactions/storage/mst_storage_impl.hpp"

auto log_ = logger::log("MstProcessorTest");
using namespace std;
using namespace iroha;
using namespace iroha::model;

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
  Peer own_peer = makePeer("iam", "1");
  shared_ptr<network::MstTransport> transport;
  shared_ptr<MstStorage> storage;
  shared_ptr<PropagationStrategy> propagation_strategy;
  shared_ptr<MstTimeProvider> time_provider;
  shared_ptr<FairMstProcessor> mst_processor;

 protected:
  void SetUp() override {
    transport = make_shared<MockMstTransport>();
    storage = make_shared<MstStorageStateImpl>(own_peer,
                                               make_shared<TestCompleter>());
    propagation_strategy = make_shared<MockPropagationStrategy>();
    time_provider = make_shared<MockTimeProvider>();

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
 * state not update
 * AND absent prepared transactions
 * AND absent expired transactions
 */
TEST_F(MstProcessorTest, InitializationTes) {
  ASSERT_NE(nullptr, mst_processor);
}
