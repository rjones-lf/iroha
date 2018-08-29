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

#ifndef IROHA_TRANSACTION_PROCESSOR_STUB_HPP
#define IROHA_TRANSACTION_PROCESSOR_STUB_HPP

#include <mutex>

#include "builders/default_builders.hpp"
#include "interfaces/transaction_responses/tx_response.hpp"
#include "logger/logger.hpp"
#include "multi_sig_transactions/mst_processor.hpp"
#include "network/peer_communication_service.hpp"
#include "torii/processor/transaction_processor.hpp"
#include "torii/status_bus.hpp"

namespace iroha {
  namespace torii {
    class TransactionProcessorImpl : public TransactionProcessor {
     public:
      /**
       * @param pcs - provide information proposals and commits
       * @param mst_processor is a handler for multisignature transactions
       * @param status_bus is a common notifier for tx statuses
       */
      TransactionProcessorImpl(
          std::shared_ptr<network::PeerCommunicationService> pcs,
          std::shared_ptr<MstProcessor> mst_processor,
          std::shared_ptr<iroha::torii::StatusBus> status_bus);

      void batchHandle(const shared_model::interface::TransactionBatch
                           &transaction_batch) const override;

     private:
      // connections
      std::shared_ptr<network::PeerCommunicationService> pcs_;

      // processing
      std::shared_ptr<MstProcessor> mst_processor_;

      std::shared_ptr<iroha::torii::StatusBus> status_bus_;

      // internal
      rxcpp::subjects::subject<
          std::shared_ptr<shared_model::interface::TransactionResponse>>
          notifier_;

      // keeps hashes of transaction, which were committed during this round
      std::vector<shared_model::interface::types::HashType> current_txs_hashes_;

      logger::Logger log_;

      /// prevents from emitting new tx statuses from different threads
      /// in parallel
      std::mutex notifier_mutex_;
    };
  }  // namespace torii
}  // namespace iroha

#endif  // IROHA_TRANSACTION_PROCESSOR_STUB_HPP
