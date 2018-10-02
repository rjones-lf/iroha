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
     public:
      using FactoryReturnType = interface::TxStatusFactory::FactoryReturnType;

      // ------------------------| Stateless statuses |-------------------------

      FactoryReturnType makeStatelessFail(TransactionHashType,
                                          ErrorMessageType) override;

      FactoryReturnType makeStatelessValid(TransactionHashType,
                                           ErrorMessageType) override;

      // ------------------------| Stateful statuses |--------------------------

      FactoryReturnType makeStatefulFail(TransactionHashType,
                                         ErrorMessageType) override;
      FactoryReturnType makeStatefulValid(TransactionHashType,
                                          ErrorMessageType) override;

      // --------------------------| Final statuses |---------------------------

      FactoryReturnType makeCommitted(TransactionHashType,
                                      ErrorMessageType) override;

      FactoryReturnType makeRejected(TransactionHashType,
                                     ErrorMessageType) override;

      // --------------------------| Rest statuses |----------------------------

      FactoryReturnType makeMstExpired(TransactionHashType,
                                       ErrorMessageType) override;

      FactoryReturnType makeNotReceived(TransactionHashType,
                                        ErrorMessageType) override;

      FactoryReturnType makeEnoughSignaturesCollected(
          TransactionHashType, ErrorMessageType) override;
    };
  }  // namespace proto
}  // namespace shared_model
#endif  // IROHA_PROTO_TX_STATUS_FACTORY_HPP
