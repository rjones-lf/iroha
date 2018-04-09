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

#ifndef IROHA_MODEL_MOCKS_HPP
#define IROHA_MODEL_MOCKS_HPP

#include <gmock/gmock.h>
#include "ametsuchi/wsv_command.hpp"
#include "ametsuchi/wsv_query.hpp"
#include "model/command.hpp"
#include "model/query_execution.hpp"

namespace iroha {
  namespace model {

    class MockCommand : public Command {
     public:
      MOCK_METHOD2(validate, bool(ametsuchi::WsvQuery &, const Account &));
      MOCK_METHOD2(execute,
                   bool(ametsuchi::WsvQuery &, ametsuchi::WsvCommand &));

      MOCK_CONST_METHOD1(Equals, bool(const Command &));
      bool operator==(const Command &rhs) const override {
        return Equals(rhs);
      }

      MOCK_CONST_METHOD1(NotEquals, bool(const Command &));
      bool operator!=(const Command &rhs) const override {
        return NotEquals(rhs);
      }
    };

    class MockQueryProcessingFactory : public QueryProcessingFactory {
     public:
      MOCK_METHOD1(execute, std::shared_ptr<QueryResponse>(const Query &query));
    };
  }  // namespace model
}  // namespace iroha

#endif  // IROHA_MODEL_MOCKS_HPP
