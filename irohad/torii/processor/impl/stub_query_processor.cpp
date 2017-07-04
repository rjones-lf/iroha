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

#include <torii/processor/stub_query_processor.hpp>

namespace iroha {
  namespace torii {
    using rxcpp::subscriber;
    using std::shared_ptr;
    using dao::Query;
    using dao::QueryResponse;
    using dao::Client;

    QueryProcessorStub::QueryProcessorStub(ametsuchi::WsvQuery &wsv,
                                           ametsuchi::BlockQuery &block) :
        wsv_(wsv), block_(block) {
      handler_.insert<dao::GetBlocks>([this](const auto &query) {
        return this->handle_get_blocks(query);
      });
    }

    void QueryProcessorStub::query_handle(dao::Client client,
                                          const dao::Query &query) {
      std::cout << "[QueryProcessor] find handler... ";
      auto handle = handler_.find(query).value_or([](auto &) {
        std::cout << "handler not found" << std::endl;
        return;
      });
      handle(query);
      return;
    }

    rxcpp::observable<shared_ptr<QueryResponse>>
    QueryProcessorStub::query_notifier() {
      return subject_.get_observable();
    }

    void QueryProcessorStub::handle_get_blocks(const dao::GetBlocks &blocks) {
      std::cout << "handler found - get_blocks()" << std::endl;
      block_.get_blocks_in_range(blocks.from, blocks.to);
      subject_.get_subscriber().on_next(
          std::make_shared<dao::GetBlocksResponse>());
    }

  }  // namespace torii
}  // namespace iroha
