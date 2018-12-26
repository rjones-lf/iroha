/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "torii/processor/query_processor_impl.hpp"

#include <boost/range/size.hpp>

#include "ametsuchi/wsv_query.hpp"
#include "common/bind.hpp"
#include "interfaces/queries/blocks_query.hpp"
#include "interfaces/queries/query.hpp"
#include "interfaces/query_responses/block_query_response.hpp"
#include "interfaces/query_responses/query_response.hpp"
#include "validation/utils.hpp"

namespace iroha {
  namespace torii {

    QueryProcessorImpl::QueryProcessorImpl(
        std::shared_ptr<ametsuchi::Storage> storage,
        std::shared_ptr<ametsuchi::QueryExecutorFactory> qry_exec,
        std::shared_ptr<iroha::PendingTransactionStorage> pending_transactions,
        std::shared_ptr<shared_model::interface::QueryResponseFactory>
            response_factory,
        logger::Logger log)
        : storage_{std::move(storage)},
          qry_exec_{std::move(qry_exec)},
          pending_transactions_{std::move(pending_transactions)},
          response_factory_{std::move(response_factory)},
          log_{std::move(log)} {
      storage_->on_commit().subscribe(
          [this](std::shared_ptr<shared_model::interface::Block> block) {
            auto block_response =
                response_factory_->createBlockQueryResponse(clone(*block));
            blocks_query_subject_.get_subscriber().on_next(
                std::move(block_response));
          });
    }

    template <class Q>
    bool QueryProcessorImpl::checkSignatories(const Q &qry) {
      const auto &wsv_query = storage_->getWsvQuery();

      auto signatories = wsv_query->getSignatories(qry.creatorAccountId());
      const auto &sig = qry.signatures();

      return boost::size(sig) == 1
          and signatories | [&sig](const auto &signatories) {
                return validation::signaturesSubset(sig, signatories);
              };
    }

    template bool QueryProcessorImpl::checkSignatories<
        shared_model::interface::Query>(const shared_model::interface::Query &);
    template bool
    QueryProcessorImpl::checkSignatories<shared_model::interface::BlocksQuery>(
        const shared_model::interface::BlocksQuery &);

    std::unique_ptr<shared_model::interface::QueryResponse>
    QueryProcessorImpl::queryHandle(const shared_model::interface::Query &qry) {
      if (not checkSignatories(qry)) {
        // TODO [IR-1816] Akvinikym 03.12.18: replace magic number 3
        // with a named constant
        return response_factory_->createErrorQueryResponse(
            shared_model::interface::QueryResponseFactory::ErrorQueryType::
                kStatefulFailed,
            "query signatories did not pass validation",
            3,
            qry.hash());
      }

      auto executor = qry_exec_->createQueryExecutor(pending_transactions_,
                                                     response_factory_);
      if (not executor) {
        log_->error("Cannot create query executor");
        return nullptr;
      }

      return executor.value()->validateAndExecute(qry);
    }

    rxcpp::observable<
        std::shared_ptr<shared_model::interface::BlockQueryResponse>>
    QueryProcessorImpl::blocksQueryHandle(
        const shared_model::interface::BlocksQuery &qry) {
      if (not checkSignatories(qry)) {
        std::shared_ptr<shared_model::interface::BlockQueryResponse> response =
            response_factory_->createBlockQueryResponse(
                "query signatories did not pass validation");
        return rxcpp::observable<>::just(std::move(response));
      }

      auto exec = qry_exec_->createQueryExecutor(pending_transactions_,
                                                 response_factory_);
      if (not exec or not(exec | [&qry](const auto &executor) {
            return executor->validate(qry);
          })) {
        std::shared_ptr<shared_model::interface::BlockQueryResponse> response =
            response_factory_->createBlockQueryResponse("stateful invalid");
        return rxcpp::observable<>::just(response);
      }
      return blocks_query_subject_.get_observable();
    }

  }  // namespace torii
}  // namespace iroha
