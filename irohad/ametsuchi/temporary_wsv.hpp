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

#ifndef IROHA_TEMPORARYWSV_HPP
#define IROHA_TEMPORARYWSV_HPP

#include <functional>

#include "ametsuchi/wsv_command.hpp"
#include "ametsuchi/wsv_query.hpp"
#include "validation/stateful_validator_common.hpp"

namespace shared_model {
  namespace interface {
    class Transaction;
  }  // namespace interface
}  // namespace shared_model

namespace iroha {
  namespace ametsuchi {

    /**
     * Temporary world state view
     * Allows to query the temporary world state view
     */
    class TemporaryWsv {
     public:
      /**
       * Wrapper for savepoints in wsv state; rollbacks to savepoint, if
       * destroyed without explicit release, releases it otherwise
       */
      struct SavepointWrapper {
        /**
         * Release the savepoint
         */
        virtual void release() = 0;

        virtual ~SavepointWrapper() = default;
      };

      /**
       * Applies a transaction to current state
       * using logic specified in function
       * @param transaction Transaction to be applied
       * @param function Function that specifies the logic used to apply the
       * transaction
       * @return True if transaction was successfully applied, false otherwise
       */
      virtual expected::Result<void, validation::CommandError> apply(
          const shared_model::interface::Transaction &,
          std::function<expected::Result<void, validation::CommandError>(
              const shared_model::interface::Transaction &, WsvQuery &)>
              function) = 0;

      /**
       * Create a savepoint for wsv state
       * @param name of savepoint to be created
       * @return RAII wrapper for savepoints
       */
      virtual std::unique_ptr<TemporaryWsv::SavepointWrapper> createSavepoint(
          const std::string &name) = 0;

      virtual ~TemporaryWsv() = default;

    };
  }     // namespace ametsuchi
}  // namespace iroha

#endif  // IROHA_TEMPORARYWSV_HPP
