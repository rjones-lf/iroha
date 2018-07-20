/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "torii/processor/query_processor_impl.hpp"

#include <boost/range/size.hpp>

#include "ametsuchi/wsv_query.hpp"
#include "builders/protobuf/builder_templates/query_response_template.hpp"
#include "builders/protobuf/query_responses/proto_block_query_response_builder.hpp"
#include "interfaces/queries/blocks_query.hpp"
#include "interfaces/queries/query.hpp"
#include "interfaces/query_responses/block_query_response.hpp"
#include "interfaces/query_responses/query_response.hpp"
#include "validation/utils.hpp"

namespace iroha {
  namespace torii {
    /**
     * Builds QueryResponse that contains StatefulError
     * @param hash - original query hash
     * @return QueryRepsonse
     */
    auto buildStatefulError(
        const shared_model::interface::types::HashType &hash) {
      return clone(
          shared_model::proto::TemplateQueryResponseBuilder<>()
              .queryHash(hash)
              .errorQueryResponse<
                  shared_model::interface::StatefulFailedErrorResponse>()
              .build());
    }
    std::shared_ptr<shared_model::interface::BlockQueryResponse>
    buildBlocksQueryError(const std::string &message) {
      return clone(shared_model::proto::BlockQueryResponseBuilder()
                       .errorResponse(message)
                       .build());
    }

    std::shared_ptr<shared_model::interface::BlockQueryResponse>
    buildBlocksQueryBlock(shared_model::interface::Block &block) {
      return clone(shared_model::proto::BlockQueryResponseBuilder()
                       .blockResponse(block)
                       .build());
    }

    QueryProcessorImpl::QueryProcessorImpl(
        std::shared_ptr<ametsuchi::Storage> storage,
        std::shared_ptr<QueryExecution> qry_exec)
        : storage_(storage), qry_exec_(qry_exec) {
      storage_->on_commit().subscribe(
          [this](std::shared_ptr<shared_model::interface::Block> block) {
            auto response = buildBlocksQueryBlock(*block);
            blocks_query_subject_.get_subscriber().on_next(response);
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
        return buildStatefulError(qry.hash());
      }

      return qry_exec_->validateAndExecute(qry);
    }

    rxcpp::observable<
        std::shared_ptr<shared_model::interface::BlockQueryResponse>>
    QueryProcessorImpl::blocksQueryHandle(
        const shared_model::interface::BlocksQuery &qry) {
      if (not checkSignatories(qry)) {
        auto response = buildBlocksQueryError("Wrong signatories");
        return rxcpp::observable<>::just(response);
      }

      if (not qry_exec_->validate(qry)) {
        auto response = buildBlocksQueryError("Stateful invalid");
        return rxcpp::observable<>::just(response);
      }
      return blocks_query_subject_.get_observable();
    }

  }  // namespace torii
}  // namespace iroha
