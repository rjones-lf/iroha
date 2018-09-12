/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_PROTO_TX_STATUS_FACTORY_HPP
#define IROHA_PROTO_TX_STATUS_FACTORY_HPP

#include "backend/protobuf/transaction_responses/proto_tx_response.hpp"
#include "interfaces/iroha_internal/tx_status_factory.hpp"

namespace shared_model {
  namespace proto {
    class ProtoTxStatusFactory : public interface::TxStatusFactory {
     private:
      using FactoryReturnType = interface::TxStatusFactory::FactoryReturnType;

     public:
      // ------------------------| Stateless statuses |-------------------------

      FactoryReturnType makeTxStatusStatelessFail(TransactionHashType,
                                                  ErrorMessageType) override;

      FactoryReturnType makeTxStatusStatelessValid(TransactionHashType,
                                                   ErrorMessageType) override;

      // ------------------------| Stateful statuses |--------------------------

      FactoryReturnType makeTxStatusStatefulFail(TransactionHashType,
                                                 ErrorMessageType) override;
      FactoryReturnType makeTxStatusStatefulValid(TransactionHashType,
                                                  ErrorMessageType) override;

      // ---------------------------| End statuses |----------------------------

      FactoryReturnType makeTxStatusCommitted(TransactionHashType,
                                              ErrorMessageType) override;

      FactoryReturnType makeTxStatusRejected(TransactionHashType,
                                             ErrorMessageType) override;

      // --------------------------| Rest statuses |----------------------------

      FactoryReturnType makeTxStatusMstExpired(TransactionHashType,
                                               ErrorMessageType) override;

      FactoryReturnType makeTxStatusNotReceived(TransactionHashType,
                                                ErrorMessageType) override;

      FactoryReturnType makeEnoughSignaturesCollected(
          TransactionHashType, ErrorMessageType) override;

     private:
      /**
       * Fills common fields for all statuses
       */
      iroha::protocol::ToriiResponse fillCommon(TransactionHashType,
                                                ErrorMessageType,
                                                iroha::protocol::TxStatus);

      /**
       * Wraps status with model object
       */
      FactoryReturnType wrap(iroha::protocol::ToriiResponse &&);
    };
  }  // namespace proto
}  // namespace shared_model
#endif  // IROHA_PROTO_TX_STATUS_FACTORY_HPP
