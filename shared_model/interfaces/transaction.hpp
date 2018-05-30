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

#ifndef IROHA_SHARED_MODEL_TRANSACTION_HPP
#define IROHA_SHARED_MODEL_TRANSACTION_HPP

#include <vector>

#include "interfaces/base/signable.hpp"
#include "interfaces/commands/command.hpp"
#include "interfaces/common_objects/types.hpp"
#include "utils/polymorphic_wrapper.hpp"
#include "utils/string_builder.hpp"

namespace shared_model {
  namespace interface {

    /**
     * Transaction class represent well-formed intent from client to change
     * state of ledger.
     */
    class Transaction : public Signable<Transaction> {
     public:
      /**
       * @return creator of transaction
       */
      virtual const types::AccountIdType &creatorAccountId() const = 0;

      /// Type of quorum
      using QuorumType = uint8_t;

      /**
       * @return quorum of transaction
       */
      virtual QuorumType quorum() const = 0;

      /// Type of command
      using CommandType = detail::PolymorphicWrapper<Command>;

      /// Type of ordered collection of commands
      using CommandsType = std::vector<CommandType>;

      /**
       * @return attached commands
       */
      virtual const CommandsType &commands() const = 0;

      std::string toString() const override {
        return detail::PrettyStringBuilder()
            .init("Transaction")
            .append("hash", hash().hex())
            .append("creatorAccountId", creatorAccountId())
            .append("createdTime", std::to_string(createdTime()))
            .append("quorum", std::to_string(quorum()))
            .append("commands")
            .appendAll(commands(),
                       [](auto &command) { return command->toString(); })
            .append("signatures")
            .appendAll(signatures(), [](auto &sig) { return sig.toString(); })
            .finalize();
      }
    };

  }  // namespace interface
}  // namespace shared_model
#endif  // IROHA_SHARED_MODEL_TRANSACTION_HPP
