/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_QUERY_PROCESSOR_IMPL_HPP
#define IROHA_QUERY_PROCESSOR_IMPL_HPP

#include "ametsuchi/storage.hpp"
#include "logger/logger.hpp"
#include "torii/processor/query_processor.hpp"

namespace iroha {
  namespace torii {

    /**
     * QueryProcessorImpl provides implementation of QueryProcessor
     */
    class QueryProcessorImpl : public QueryProcessor {
     public:
      QueryProcessorImpl(
          std::shared_ptr<ametsuchi::Storage> storage,
          std::shared_ptr<ametsuchi::QueryExecutorFactory> qry_exec,
          std::shared_ptr<iroha::PendingTransactionStorage>
              pending_transactions);

      /**
       * Checks if query has needed signatures
       * @param qry arrived query
       * @return true if passes stateful validation
       */
      template <class Q>
      bool checkSignatories(const Q &qry);

      std::unique_ptr<shared_model::interface::QueryResponse> queryHandle(
          const shared_model::interface::Query &qry) override;

      rxcpp::observable<
          std::shared_ptr<shared_model::interface::BlockQueryResponse>>
      blocksQueryHandle(
          const shared_model::interface::BlocksQuery &qry) override;

     private:
      rxcpp::subjects::subject<
          std::shared_ptr<shared_model::interface::BlockQueryResponse>>
          blocks_query_subject_;
      std::shared_ptr<ametsuchi::Storage> storage_;
      std::shared_ptr<ametsuchi::QueryExecutorFactory> qry_exec_;
      std::shared_ptr<iroha::PendingTransactionStorage> pending_transactions_;

      logger::Logger log_;
    };

  }  // namespace torii
}  // namespace iroha

#endif  // IROHA_QUERY_PROCESSOR_IMPL_HPP
