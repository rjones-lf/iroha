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

#ifndef IROHA_TRANSACTION_GENERATOR_HPP
#define IROHA_TRANSACTION_GENERATOR_HPP

#include "model/generators/command_generator.hpp"
#include "model/generators/signature_generator.hpp"
#include "model/transaction.hpp"

namespace iroha {
  namespace model {
    namespace generators {
      class TransactionGenerator {
       public:
        /**
         * Generate genesis transaction, contain all necessary commands for new
         * Iroha network
         * @param timestamp
         * @param peers_address
         * @return
         */
        Transaction generateGenesisTransaction(
            ts64_t timestamp, std::vector<std::string> peers_address);

        /**
         * Generate transaction from give meta data and commands list
         * @param timestamp
         * @param creator_account_id
         * @param commands
         * @return
         */
        Transaction generateTransaction(
            ts64_t timestamp,
            std::string creator_account_id,
            std::vector<std::shared_ptr<Command>> commands);

        /**
         * Generate transaction from give meta data and commands list
         * @param timestamp
         * @param creator_account_id
         * @param commands
         * @return
         */
        Transaction generateTransaction(
            std::string creator_account_id,
            std::vector<std::shared_ptr<Command>> commands);
      };
    }  // namespace generators
  }    // namespace model
}  // namespace iroha

#endif  // IROHA_TRANSACTION_GENERATOR_HPP
