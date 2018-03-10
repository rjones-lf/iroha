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

#include "logger/logger.hpp"
#include "model/transaction_response.hpp"
#include "model/types.hpp"
#include "multi_sig_transactions/mst_processor.hpp"
#include "network/peer_communication_service.hpp"
#include "torii/processor/transaction_processor.hpp"
#include "validation/stateless_validator.hpp"

namespace iroha {
  namespace torii {
    class TransactionProcessorImpl : public TransactionProcessor {
     public:
      /**
       * @param pcs - provide information proposals and commits
       * @param os - ordering service for sharing transactions
       * @param validator - perform stateless validation
       * @param crypto_provider - sign income transactions
       */
      TransactionProcessorImpl(
          std::shared_ptr<network::PeerCommunicationService> pcs,
          std::shared_ptr<validation::StatelessValidator> validator,
          std::shared_ptr<MstProcessor> mst_processor);

      void transactionHandle(ConstRefTransaction transaction) override;

      rxcpp::observable<TxResponse> transactionNotifier() override;

     private:
      // connections
      std::shared_ptr<network::PeerCommunicationService> pcs_;

      // processing
      std::shared_ptr<validation::StatelessValidator> validator_;

      std::shared_ptr<MstProcessor> mst_processor_;

      std::unordered_set<std::string> proposal_set_;
      std::unordered_set<std::string> candidate_set_;

      // internal
      rxcpp::subjects::subject<TxResponse> notifier_;

      logger::Logger log_;

      /// Wrapper on notifying on some hash
      void notify(const std::string &hash,
                  model::TransactionResponse::Status s);
    };
  }  // namespace torii
}  // namespace iroha

#endif  // IROHA_TRANSACTION_PROCESSOR_STUB_HPP
