/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_TX_STATUS_FACTORY_HPP
#define IROHA_TX_STATUS_FACTORY_HPP

#include <memory>

#include "interfaces/transaction_responses/tx_response.hpp"

namespace shared_model {
  namespace interface {

    /**
     * Factory which creates transaction status response
     */
    class TxStatusFactory {
     public:
      virtual ~TxStatusFactory() = default;

      using FactoryReturnType = std::unique_ptr<TransactionResponse>;

      using TransactionHashType =
          const TransactionResponse::TransactionHashType &;

      using ErrorMessageType = const TransactionResponse::ErrorMessageType &;

      // ------------------------| Stateless statuses |-------------------------

      /// Creates stateless failed transaction status
      virtual FactoryReturnType makeTxStatusStatelessFail(TransactionHashType,
                                                          ErrorMessageType) = 0;

      /// Creates stateless valid transaction status
      virtual FactoryReturnType makeTxStatusStatelessValid(
          TransactionHashType, ErrorMessageType) = 0;

      // ------------------------| Stateful statuses |--------------------------

      /// Creates stateful failed transaction status
      virtual FactoryReturnType makeTxStatusStatefulFail(TransactionHashType,
                                                         ErrorMessageType) = 0;
      /// Creates stateful valid transaction status
      virtual FactoryReturnType makeTxStatusStatefulValid(TransactionHashType,
                                                          ErrorMessageType) = 0;

      // ---------------------------| End statuses |----------------------------

      /// Creates committed transaction status
      virtual FactoryReturnType makeTxStatusCommitted(TransactionHashType,
                                                      ErrorMessageType) = 0;

      /// Creates rejected transaction status
      virtual FactoryReturnType makeTxStatusRejected(TransactionHashType,
                                                     ErrorMessageType) = 0;

      // --------------------------| Rest statuses |----------------------------

      /// Creates transaction expired status
      virtual FactoryReturnType makeTxStatusMstExpired(TransactionHashType,
                                                       ErrorMessageType) = 0;

      /// Creates transaction is not received status
      virtual FactoryReturnType makeTxStatusNotReceived(TransactionHashType,
                                                        ErrorMessageType) = 0;

      // Creates status which shows that enough signatures were collected
      virtual FactoryReturnType makeEnoughSignaturesCollected(
          TransactionHashType, ErrorMessageType) = 0;
    };
  }  // namespace interface
}  // namespace shared_model

#endif  // IROHA_TX_STATUS_FACTORY_HPP
