/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_EXECUTION_MOCKS
#define IROHA_EXECUTION_MOCKS

#include <gmock/gmock.h>
#include "execution/query_execution.hpp"

class MockQueryExecution : public iroha::QueryExecution {
 public:
  MOCK_METHOD1(validateAndExecute,
               std::unique_ptr<shared_model::interface::QueryResponse>(
                   const shared_model::interface::Query &));
  MOCK_METHOD1(validate, bool(const shared_model::interface::BlocksQuery &));
};

#endif  // IROHA_EXECUTION_MOCKS
