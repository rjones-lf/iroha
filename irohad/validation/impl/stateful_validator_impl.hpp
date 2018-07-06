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
#ifndef IROHA_STATEFUL_VALIDATIOR_IMPL_HPP
#define IROHA_STATEFUL_VALIDATIOR_IMPL_HPP

#include "validation/stateful_validator.hpp"

#include "logger/logger.hpp"

namespace iroha {
  namespace validation {

    /**
     * Interface for performing stateful validation
     */
    class StatefulValidatorImpl : public StatefulValidator {
     public:
      StatefulValidatorImpl();

      VerifiedProposalAndErrors validate(
          const shared_model::interface::Proposal &proposal,
          ametsuchi::TemporaryWsv &temporaryWsv) override;

      logger::Logger log_;

     private:
      /**
       * Memorizes the savepoint and batch in case it was the first tx in batch;
       * checks, if tx is from the failed batch
       * @param tx we process
       * @param temporaryWsv we use to manage savepoints
       * @return if tx does not belong to the failed batch
       */
      bool batchlyProcessTx(const shared_model::interface::Transaction &tx,
                            ametsuchi::TemporaryWsv &temporaryWsv);

      /**
       * Dismisses the batch and savepoint, if it was the last tx in the batch
       * @param tx, which successfully was applied
       * @param temporaryWsv we use to manage savepoints
       */
      void processSuccessTx(const shared_model::interface::Transaction &tx,
                            ametsuchi::TemporaryWsv &temporaryWsv);

      /**
       * Cancels all txs before the failed one and fails the current batch
       * @param tx, which failed the apply process
       * @param temporaryWsv we use to manage savepoints
       */
      void processFailedTx(const shared_model::interface::Transaction &tx,
                           ametsuchi::TemporaryWsv &temporaryWsv);

      // keeps information for tx batches processing
      struct BatchData {
        explicit BatchData(const shared_model::interface::Transaction &tx)
            : savepoint_name{tx.hash().hex().substr(0, 5)},
              last_batch_tx{
                  std::make_shared<shared_model::interface::types::HashType>(
                      tx.batch_meta()->get()->transactionHashes().back())},
              batch_failed{false} {};

        std::string savepoint_name;
        std::shared_ptr<shared_model::interface::types::HashType> last_batch_tx;
        bool batch_failed;

        bool isLastTxInBatch(const shared_model::interface::Transaction &tx) {
          return *last_batch_tx == tx.hash();
        }
      };
      std::shared_ptr<BatchData> current_atomic_batch_;
    };

  }  // namespace validation
}  // namespace iroha

#endif  // IROHA_STATEFUL_VALIDATION_IMPL_HPP
