/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_TORII_MOCKS_HPP
#define IROHA_TORII_MOCKS_HPP

#include <gmock/gmock.h>

#include "interfaces/query_responses/block_query_response.hpp"
#include "interfaces/query_responses/query_response.hpp"
#include "torii/processor/query_processor.hpp"
#include "torii/status_bus.hpp"

namespace iroha {
  namespace torii {

    class MockQueryProcessor : public QueryProcessor {
     public:
      MOCK_METHOD1(queryHandle,
                   std::unique_ptr<shared_model::interface::QueryResponse>(
                       const shared_model::interface::Query &));
      MOCK_METHOD1(
          blocksQueryHandle,
          rxcpp::observable<
              std::shared_ptr<shared_model::interface::BlockQueryResponse>>(
              const shared_model::interface::BlocksQuery &));
    };

    class MockStatusBus : public StatusBus {
     public:
      MOCK_METHOD1(publish, void(StatusBus::Objects));
      MOCK_METHOD0(statuses, rxcpp::observable<StatusBus::Objects>());
    };
  }  // namespace torii
}  // namespace iroha

#endif  // IROHA_TORII_MOCKS_HPP
