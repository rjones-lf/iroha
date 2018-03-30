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
        std::shared_ptr<ametsuchi::Storage> storage)
        : storage_(storage) {}

    void QueryProcessorImpl::queryHandle(
        std::shared_ptr<shared_model::interface::Query> qry) {
      auto qpf = model::QueryProcessingFactory(storage_->getWsvQuery(),
                                               storage_->getBlockQuery());
      auto qpf_response = qpf.execute(*qry);
      auto qry_resp =
          std::static_pointer_cast<shared_model::proto::QueryResponse>(
              qpf_response);
      subject_.get_subscriber().on_next(
          std::make_shared<shared_model::proto::QueryResponse>(
              qry_resp->getTransport()));
    }
    rxcpp::observable<std::shared_ptr<shared_model::interface::QueryResponse>>
    QueryProcessorImpl::queryNotifier() {
      return subject_.get_observable();
    }

  }  // namespace torii
}  // namespace iroha
