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

#ifndef IROHA_COMMAND_EXECUTOR_FACTORY_HPP
#define IROHA_COMMAND_EXECUTOR_FACTORY_HPP

#include <memory>
#include <boost/optional.hpp>
#include <typeindex>
#include <unordered_map>

#include "model/execution/command_executor.hpp"

namespace iroha {
  namespace model {
    /**
     * Contains map of command executors for commands used in the system
     */
    class CommandExecutorFactory {
     public:
      /**
       * Create factory by initializing the map and validating it against
       * command registry
       * @return CommandExecutorFactory object, if initialization is
       * successful
       */
      static boost::optional<std::shared_ptr<CommandExecutorFactory>> create();

      /**
       * Retrieve command executor for a given command from the map
       * @param command - command for retrieving command executor
       * @return command executor for a given command
       */
      std::shared_ptr<CommandExecutor> getCommandExecutor(
          std::shared_ptr<Command> command);

     private:
      /**
       * Initialize factory with given command executor map
       */
      explicit CommandExecutorFactory(
          std::unordered_map<std::type_index,
                             std::shared_ptr<CommandExecutor>>);

      std::unordered_map<std::type_index, std::shared_ptr<CommandExecutor>>
          executors_;
    };
  }  // namespace model
}  // namespace iroha

#endif  // IROHA_COMMAND_EXECUTOR_FACTORY_HPP
