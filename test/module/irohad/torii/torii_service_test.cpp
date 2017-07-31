/*
Copyright Soramitsu Co., Ltd. 2016 All Rights Reserved.

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

#include <endpoint.pb.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <atomic>
#include <chrono>
#include <main/server_runner.hpp>
#include <memory>
#include <thread>
#include <torii/command_client.hpp>
#include <torii/command_service.hpp>
#include <torii/processor/query_processor_impl.hpp>
#include <torii_utils/query_client.hpp>
#include <queries.pb.h>

#include "torii/processor/transaction_processor_impl.hpp"
#include "mock_classes.hpp"

constexpr const char *Ip = "0.0.0.0";
constexpr int Port = 50051;

constexpr size_t TimesToriiBlocking = 5;
constexpr size_t TimesToriiNonBlocking = 5;
constexpr size_t TimesFind = 10;

using ::testing::Return;
using ::testing::A;
using ::testing::_;
using ::testing::AtLeast;


class ToriiServiceTest : public testing::Test {
 public:
  virtual void SetUp() {
    runner = new ServerRunner(Ip, Port);
    th = std::thread([this, runner = runner] {
      // ----------- Command Service --------------

      auto tx_processor =
          iroha::torii::TransactionProcessorImpl(pcsMock, svMock);
      iroha::model::converters::PbTransactionFactory pb_tx_factory;
      auto command_service =  // std::unique_ptr<torii::CommandService>(new
                              // torii::CommandService(pb_tx_factory,
                              // tx_processor));
          std::make_unique<torii::CommandService>(pb_tx_factory, tx_processor);

      //----------- Query Service ----------
      iroha::model::QueryProcessingFactory qpf(wsv_query, block_query);

      iroha::torii::QueryProcessorImpl qpi(qpf, svMock);

      iroha::model::converters::PbQueryFactory pb_query_factory;
      iroha::model::converters::PbQueryResponseFactory pb_query_resp_factory;

      auto query_service = std::make_unique<torii::QueryService>(
          pb_query_factory, pb_query_resp_factory, qpi);

      //----------- Server run ----------------
      runner->run(std::move(command_service), std::move(query_service));
    });

    runner->waitForServersReady();
  }

  virtual void TearDown() {
    runner->shutdown();
    delete runner;
    th.join();
  }

  ServerRunner *runner;
  std::thread th;

  WsvQueryMock wsv_query;
  BlockQueryMock block_query;

  PCSMock pcsMock;
  StatelessValidatorMock svMock;
};

TEST_F(ToriiServiceTest, ToriiWhenBlocking) {
  EXPECT_CALL(svMock, validate(A<const iroha::model::Transaction &>()))
      .WillRepeatedly(Return(true));

  EXPECT_CALL(pcsMock, propagate_transaction(_)).Times(AtLeast(1));

  for (size_t i = 0; i < TimesToriiBlocking; ++i) {
    iroha::protocol::ToriiResponse response;
    // One client is generating transaction
    auto new_tx = iroha::protocol::Transaction();
    auto meta = new_tx.mutable_meta();
    meta->set_tx_counter(i);
    meta->set_creator_account_id("accountA");
    auto stat = torii::CommandSyncClient(Ip, Port).Torii(new_tx, response);
    ASSERT_TRUE(stat.ok());
    ASSERT_EQ(response.validation(),
              iroha::protocol::STATELESS_VALIDATION_SUCCESS);
  }
}

TEST_F(ToriiServiceTest, ToriiWhenBlockingInvalid) {
  EXPECT_CALL(svMock, validate(A<const iroha::model::Transaction &>()))
      .WillRepeatedly(Return(false));

  for (size_t i = 0; i < TimesToriiBlocking; ++i) {
    iroha::protocol::ToriiResponse response;
    // One client is generating transaction
    auto new_tx = iroha::protocol::Transaction();
    auto meta = new_tx.mutable_meta();
    meta->set_tx_counter(i);
    meta->set_creator_account_id("accountA");
    auto stat = torii::CommandSyncClient(Ip, Port).Torii(new_tx, response);
    ASSERT_TRUE(stat.ok());
    ASSERT_EQ(response.validation(),
              iroha::protocol::STATELESS_VALIDATION_FAILED);
  }
}

TEST_F(ToriiServiceTest, ToriiWhenNonBlocking) {
  torii::CommandAsyncClient client(Ip, Port);
  std::atomic_int count{0};

  EXPECT_CALL(svMock, validate(A<const iroha::model::Transaction &>()))
      .WillRepeatedly(Return(true));

  EXPECT_CALL(pcsMock, propagate_transaction(_)).Times(AtLeast(1));

  for (size_t i = 0; i < TimesToriiNonBlocking; ++i) {
    auto new_tx = iroha::protocol::Transaction();
    auto meta = new_tx.mutable_meta();
    meta->set_tx_counter(i);
    meta->set_creator_account_id("accountA");

    auto stat =
        client.Torii(new_tx, [&count](iroha::protocol::ToriiResponse response) {
          ASSERT_EQ(response.validation(),
                    iroha::protocol::STATELESS_VALIDATION_SUCCESS);
          count++;
        });
  }

  while (count < (int)TimesToriiNonBlocking)
    ;
  ASSERT_EQ(count, TimesToriiNonBlocking);
}

