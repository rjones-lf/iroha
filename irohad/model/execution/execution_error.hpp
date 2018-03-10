/**
 * Copyright Soramitsu Co., Ltd. 2018 All Rights Reserved.
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

#ifndef IROHA_EXECUTION_ERROR_HPP
#define IROHA_EXECUTION_ERROR_HPP

#include <boost/format.hpp>

namespace iroha {
  namespace model {

    /**
     * Error for command execution.
     * Contains command name, as well as an error message
     */
    struct ExecutionError {
      std::string command_name;
      std::string error_message;

      std::string toString() const {
        return (boost::format("%s: %s") % command_name % error_message).str();
      }
    };
  }  // namespace model
}  // namespace iroha

#endif  // IROHA_EXECUTION_ERROR_HPP
