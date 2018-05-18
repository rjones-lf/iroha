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

#ifndef IROHA_BLOCK_VALIDATOR_HPP
#define IROHA_BLOCK_VALIDATOR_HPP

#include "interfaces/iroha_internal/block.hpp"
#include "validators/container_fields/container_validator.hpp"
#include "validators/container_fields/transactions_validator.hpp"

namespace shared_model {
  namespace validation {

    /**
     * Class that validates block
     */
    template <typename FieldValidator, typename TransactionValidator>
    class BlockValidator
        // TODO IR-1295 10.05.18 Use NonEmptyTransactionValidator when 1295 is
        // implemented
        : public ContainerValidator<
              interface::Block,
              FieldValidator,
              TransactionsValidator<TransactionValidator>> {
     public:
      /**
       * Applies validation on block
       * @param block
       * @return Answer containing found error if any
       */
      Answer validate(const interface::Block &block) const {
        return ContainerValidator<
            interface::Block,
            FieldValidator,
            TransactionsValidator<TransactionValidator>>::validate(block,
                                                                   "Block");
      }
    };

  }  // namespace validation
}  // namespace shared_model

#endif  // IROHA_BLOCK_VALIDATOR_HPP
