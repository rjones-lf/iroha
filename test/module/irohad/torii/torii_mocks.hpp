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

#ifndef IROHA_TORII_MOCKS_HPP
#define IROHA_TORII_MOCKS_HPP

#include "torii/processor/query_processor.hpp"

#include <gmock/gmock.h>

namespace iroha {
  namespace torii {
    class MockQueryProcessor : public QueryProcessor {
     public:
      MOCK_METHOD1(queryHandle,
                   void(std::shared_ptr<shared_model::interface::Query>));
      MOCK_METHOD0(
          queryNotifier,
          rxcpp::observable<
              std::shared_ptr<shared_model::interface::QueryResponse>>());
    };
  }  // namespace torii
}  // namespace iroha

#endif  // IROHA_TORII_MOCKS_HPP
