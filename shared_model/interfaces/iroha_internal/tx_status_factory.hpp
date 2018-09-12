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
      /// return type of all generative methods
      using FactoryReturnType = std::unique_ptr<TransactionResponse>;

      /// type of transaction hash
      using TransactionHashType =
          const TransactionResponse::TransactionHashType &;

      /// type of error attached to \class TransactionResponse instance
      using ErrorMessageType = const TransactionResponse::ErrorMessageType &;

      // ------------------------| Stateless statuses |-------------------------

      /**
       * Creates stateless failed transaction status
       * @param
       */
      virtual FactoryReturnType makeStatelessFail(TransactionHashType,
                                                  ErrorMessageType) = 0;

      /// Creates stateless valid transaction status
      virtual FactoryReturnType makeStatelessValid(TransactionHashType,
                                                   ErrorMessageType) = 0;

      // ------------------------| Stateful statuses |--------------------------

      /// Creates stateful failed transaction status
      virtual FactoryReturnType makeStatefulFail(TransactionHashType,
                                                 ErrorMessageType) = 0;
      /// Creates stateful valid transaction status
      virtual FactoryReturnType makeStatefulValid(TransactionHashType,
                                                  ErrorMessageType) = 0;

      // --------------------------| Final statuses |---------------------------

      /// Creates committed transaction status
      virtual FactoryReturnType makeCommitted(TransactionHashType,
                                              ErrorMessageType) = 0;

      /// Creates rejected transaction status
      virtual FactoryReturnType makeRejected(TransactionHashType,
                                             ErrorMessageType) = 0;

      // --------------------------| Rest statuses |----------------------------

      /// Creates transaction expired status
      virtual FactoryReturnType makeMstExpired(TransactionHashType,
                                               ErrorMessageType) = 0;

      /// Creates transaction is not received status
      virtual FactoryReturnType makeNotReceived(TransactionHashType,
                                                ErrorMessageType) = 0;

      /// Creates status which shows that enough signatures were collected
      virtual FactoryReturnType makeEnoughSignaturesCollected(
          TransactionHashType, ErrorMessageType) = 0;

      virtual ~TxStatusFactory() = default;
    };
  }  // namespace interface
}  // namespace shared_model

#endif  // IROHA_TX_STATUS_FACTORY_HPP
