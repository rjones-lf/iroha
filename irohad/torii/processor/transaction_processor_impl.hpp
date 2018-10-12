/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_TRANSACTION_PROCESSOR_STUB_HPP
#define IROHA_TRANSACTION_PROCESSOR_STUB_HPP

#include "torii/processor/transaction_processor.hpp"

#include <mutex>

#include <rxcpp/rx.hpp>
#include "interfaces/common_objects/transaction_sequence_common.hpp"
#include "interfaces/transaction_responses/tx_response.hpp"
#include "logger/logger.hpp"
#include "multi_sig_transactions/mst_processor.hpp"
#include "network/peer_communication_service.hpp"
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

      void batchHandle(
          std::shared_ptr<shared_model::interface::TransactionBatch>
              transaction_batch) const override;

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

      // TODO: [IR-1665] Akvinikym 29.08.18: Refactor method publishStatus(..)
      /**
       * Complementary class for publishStatus method
       */
      enum class TxStatusType {
        kStatelessFailed,
        kStatelessValid,
        kStatefulFailed,
        kStatefulValid,
        kCommitted,
        kMstExpired,
        kNotReceived,
        kMstPending,
        kEnoughSignaturesCollected
      };
      /**
       * Publish status of transaction
       * @param tx_status to be published
       * @param hash of that transaction
       * @param error, which can appear during validation
       */
      void publishStatus(TxStatusType tx_status,
                         const shared_model::crypto::Hash &hash,
                         const std::string &error = "") const;

      /**
       * Publish kEnoughSignaturesCollected status for each transaction in
       * collection
       * @param txs - collection of those transactions
       */
      void publishEnoughSignaturesStatus(
          const shared_model::interface::types::SharedTxsCollectionType &txs)
          const;
    };
  }  // namespace torii
}  // namespace iroha

#endif  // IROHA_TRANSACTION_PROCESSOR_STUB_HPP
