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

#include "torii/processor/query_processor_impl.hpp"
#include "backend/protobuf/from_old_model.hpp"
#include "backend/protobuf/query_responses/proto_query_response.hpp"

namespace iroha {
  namespace torii {

    QueryProcessorImpl::QueryProcessorImpl(
        std::unique_ptr<model::QueryProcessingFactory> qpf)
        : qpf_(std::move(qpf)) {}

    void QueryProcessorImpl::queryHandle(
        std::shared_ptr<shared_model::interface::Query> qry) {
      std::shared_ptr<iroha::model::Query> query(qry->makeOldModel());
      // TODO: 12.02.2018 grimadas Remove when query_executor has new model, as
      // query is already stateless valid when passing to query  processor

      auto qpf_response =
          qpf_->execute(std::shared_ptr<const model::Query>(query));
      auto qry_resp = shared_model::proto::from_old(qpf_response);
      subject_.get_subscriber().on_next(
          std::make_shared<shared_model::proto::QueryResponse>(
              qry_resp.getTransport()));
    }
    rxcpp::observable<std::shared_ptr<shared_model::interface::QueryResponse>>
    QueryProcessorImpl::queryNotifier() {
      return subject_.get_observable();
    }

  }  // namespace torii
}  // namespace iroha
