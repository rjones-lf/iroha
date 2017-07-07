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

#include <validation/stateful/stub_command_validator.hpp>

namespace iroha {
  namespace validation {

    CommandValidatorStub::CommandValidatorStub(
        ametsuchi::TemporaryWsv &wsv) : wsv(wsv) {
      this->handler.insert<model::AddPeer>(std::bind(&CommandValidatorStub::validateAddPeer,
                                                   this,
                                                   std::placeholders::_1));
    }

    bool CommandValidatorStub::validate(const model::Command &command) {
      return handler.find(command).value_or([](auto &) {
        std::cout << "[CVS] handler not found" << std::endl;
        return false;
      })(command);
    }

    bool CommandValidatorStub::validateAddPeer(const model::AddPeer &addPeer) {
      return true;
    }
  } // namespace validation
} // namespace iroha

