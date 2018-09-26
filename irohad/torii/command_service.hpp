/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef TORII_COMMAND_SERVICE_HPP
#define TORII_COMMAND_SERVICE_HPP

#include <rxcpp/rx-observable.hpp>
#include "interfaces/common_objects/types.hpp"

namespace shared_model {
  namespace interface {
    class TransactionSequence;
    class TransactionResponse;
  }  // namespace interface
  namespace crypto {
    class Hash;
  }  // namespace crypto
}  // namespace shared_model
namespace torii {
  class CommandService {
   public:
    virtual ~CommandService() = default;

    /**
     * Actual implementation of sync Torii in CommandService
     * @param tx_list - transactions we've received
     */
    virtual void handleTransactionList(
        const shared_model::interface::TransactionSequence &tx_list) = 0;

    /**
     * Request to retrieve a status of any particular transaction
     * @param request - TxStatusRequest object which identifies transaction
     * uniquely
     * @return response which contains a current state of requested transaction
     */
    virtual std::shared_ptr<shared_model::interface::TransactionResponse>
    getStatus(const shared_model::crypto::Hash &request) = 0;

    /**
     * Streaming call which will repeatedly send all statuses of requested
     * transaction from its status at the moment of receiving this request to
     * the some final transaction status (which cannot change anymore)
     * @param request- TxStatusRequest object which identifies transaction
     * uniquely
     * @return observable with transaction statuses
     */
    virtual rxcpp::observable<
        std::shared_ptr<shared_model::interface::TransactionResponse>>
    getStatusStream(const shared_model::crypto::Hash &hash) = 0;
  };
}  // namespace torii

#endif  // TORII_COMMAND_SERVICE_HPP
