/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>
#include <memory>
#include "libfuzzer/libfuzzer_macro.h"
#include "module/irohad/ametsuchi/ametsuchi_mocks.hpp"
#include "module/irohad/execution/execution_mocks.hpp"
#include "module/irohad/network/network_mocks.hpp"
#include "torii/processor/query_processor_impl.hpp"
#include "torii/query_service.hpp"

using namespace std::chrono_literals;
using testing::_;
using testing::Return;

struct QueryFixture {
  std::shared_ptr<torii::QueryService> service_;
  std::shared_ptr<iroha::torii::QueryProcessorImpl> qry_processor_;
  std::shared_ptr<MockQueryExecution> qry_exec_;
  std::shared_ptr<iroha::ametsuchi::MockStorage> storage_;
  std::shared_ptr<iroha::ametsuchi::MockBlockQuery> bq_;
  std::shared_ptr<iroha::ametsuchi::MockWsvQuery> wq_;

  QueryFixture() {
    storage_ = std::make_shared<iroha::ametsuchi::MockStorage>();
    bq_ = std::make_shared<iroha::ametsuchi::MockBlockQuery>();
    wq_ = std::make_shared<iroha::ametsuchi::MockWsvQuery>();
    EXPECT_CALL(*storage_, getBlockQuery()).WillRepeatedly(Return(bq_));
    EXPECT_CALL(*storage_, getWsvQuery()).WillRepeatedly(Return(wq_));
    qry_exec_ = std::make_shared<MockQueryExecution>();
    qry_processor_ =
        std::make_shared<iroha::torii::QueryProcessorImpl>(storage_, qry_exec_);
    service_ = std::make_shared<torii::QueryService>(qry_processor_);
  }
};

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, std::size_t size) {
  static QueryFixture handler;
  if (size < 1) {
    return 0;
  }

  EXPECT_CALL(*handler.qry_exec_, validateAndExecute(_))
      .WillRepeatedly(testing::Invoke([](auto &) { return nullptr; }));
  iroha::protocol::Query qry;
  if (protobuf_mutator::libfuzzer::LoadProtoInput(true, data, size, &qry)) {
    iroha::protocol::QueryResponse resp;
    handler.service_->Find(qry, resp);
  }
  return 0;
}
