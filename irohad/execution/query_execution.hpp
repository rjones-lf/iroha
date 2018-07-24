/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_QUERY_EXECUTION_HPP
#define IROHA_QUERY_EXECUTION_HPP

#include <memory>

namespace shared_model {
  namespace interface {
    class Query;
    class BlocksQuery;
    class QueryResponse;
  }  // namespace interface
}  // namespace shared_model

namespace iroha {

  class QueryExecution {
   public:
    virtual ~QueryExecution() = default;
    /**
     * Execute and validate query.
     *
     * @param query
     * @return query response
     */
    virtual std::unique_ptr<shared_model::interface::QueryResponse>
    validateAndExecute(const shared_model::interface::Query &query) = 0;

    /**
     * Perform BlocksQuery validation
     * @param query to validate
     * @return true if valid, false otherwise
     */
    virtual bool validate(
        const shared_model::interface::BlocksQuery &query) = 0;
  };
}  // namespace iroha

#endif  // IROHA_QUERY_EXECUTION_HPP
