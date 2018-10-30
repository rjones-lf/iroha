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
                                          StatelessErrorOrFailedCommandNameType,
                                          FailedCommandIndexType,
                                          ErrorCodeType) override;

      FactoryReturnType makeStatelessValid(
          TransactionHashType,
          StatelessErrorOrFailedCommandNameType,
          FailedCommandIndexType,
          ErrorCodeType) override;

      // ------------------------| Stateful statuses |--------------------------

      FactoryReturnType makeStatefulFail(TransactionHashType,
                                         StatelessErrorOrFailedCommandNameType,
                                         FailedCommandIndexType,
                                         ErrorCodeType) override;
      FactoryReturnType makeStatefulValid(TransactionHashType,
                                          StatelessErrorOrFailedCommandNameType,
                                          FailedCommandIndexType,
                                          ErrorCodeType) override;

      // --------------------------| Final statuses |---------------------------

      FactoryReturnType makeCommitted(TransactionHashType,
                                      StatelessErrorOrFailedCommandNameType,
                                      FailedCommandIndexType,
                                      ErrorCodeType) override;

      FactoryReturnType makeRejected(TransactionHashType,
                                     StatelessErrorOrFailedCommandNameType,
                                     FailedCommandIndexType,
                                     ErrorCodeType) override;

      // --------------------------| Rest statuses |----------------------------

      FactoryReturnType makeMstExpired(TransactionHashType,
                                       StatelessErrorOrFailedCommandNameType,
                                       FailedCommandIndexType,
                                       ErrorCodeType) override;

      FactoryReturnType makeNotReceived(TransactionHashType,
                                        StatelessErrorOrFailedCommandNameType,
                                        FailedCommandIndexType,
                                        ErrorCodeType) override;

      FactoryReturnType makeEnoughSignaturesCollected(
          TransactionHashType,
          StatelessErrorOrFailedCommandNameType,
          FailedCommandIndexType,
          ErrorCodeType) override;
    };
  }  // namespace proto
}  // namespace shared_model
#endif  // IROHA_PROTO_TX_STATUS_FACTORY_HPP
