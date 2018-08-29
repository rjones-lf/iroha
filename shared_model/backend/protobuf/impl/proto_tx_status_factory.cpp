/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "backend/protobuf/proto_tx_status_factory.hpp"

using namespace shared_model::proto;

// ---------------------------| Stateless statuses |----------------------------

ProtoTxStatusFactory::FactoryReturnType
ProtoTxStatusFactory::makeTxStatusStatelessFail(TransactionHashType hash,
                                                ErrorMessageType error) {
  return wrap(fillCommon(
      hash, error, iroha::protocol::TxStatus::STATELESS_VALIDATION_FAILED));
}

ProtoTxStatusFactory::FactoryReturnType
ProtoTxStatusFactory::makeTxStatusStatelessValid(TransactionHashType hash,
                                                 ErrorMessageType error) {
  return wrap(fillCommon(
      hash, error, iroha::protocol::TxStatus::STATELESS_VALIDATION_SUCCESS));
}

// ---------------------------| Stateful statuses |-----------------------------

ProtoTxStatusFactory::FactoryReturnType
ProtoTxStatusFactory::makeTxStatusStatefulFail(TransactionHashType hash,
                                               ErrorMessageType error) {
  return wrap(fillCommon(
      hash, error, iroha::protocol::TxStatus::STATEFUL_VALIDATION_FAILED));
}
ProtoTxStatusFactory::FactoryReturnType
ProtoTxStatusFactory::makeTxStatusStatefulValid(TransactionHashType hash,
                                                ErrorMessageType error) {
  return wrap(fillCommon(
      hash, error, iroha::protocol::TxStatus::STATEFUL_VALIDATION_SUCCESS));
}

// ------------------------------| End statuses |-------------------------------

ProtoTxStatusFactory::FactoryReturnType
ProtoTxStatusFactory::makeTxStatusCommitted(TransactionHashType hash,
                                            ErrorMessageType error) {
  return wrap(fillCommon(hash, error, iroha::protocol::TxStatus::COMMITTED));
}

ProtoTxStatusFactory::FactoryReturnType
ProtoTxStatusFactory::makeTxStatusRejected(TransactionHashType hash,
                                           ErrorMessageType error) {
  return wrap(fillCommon(hash, error, iroha::protocol::TxStatus::REJECTED));
}

// -----------------------------| Rest statuses |-------------------------------

ProtoTxStatusFactory::FactoryReturnType
ProtoTxStatusFactory::makeTxStatusMstExpired(TransactionHashType hash,
                                             ErrorMessageType error) {
  return wrap(fillCommon(hash, error, iroha::protocol::TxStatus::MST_EXPIRED));
}

ProtoTxStatusFactory::FactoryReturnType
ProtoTxStatusFactory::makeTxStatusNotReceived(TransactionHashType hash,
                                              ErrorMessageType error) {
  return wrap(fillCommon(hash, error, iroha::protocol::TxStatus::NOT_RECEIVED));
}

// -------------------------------| Private API |-------------------------------

iroha::protocol::ToriiResponse ProtoTxStatusFactory::fillCommon(
    TransactionHashType hash,
    ErrorMessageType error,
    iroha::protocol::TxStatus status) {
  iroha::protocol::ToriiResponse response;
  response.set_tx_hash(crypto::toBinaryString(hash));
  response.set_error_message(error);
  response.set_tx_status(status);
  return response;
}

ProtoTxStatusFactory::FactoryReturnType ProtoTxStatusFactory::wrap(
    iroha::protocol::ToriiResponse &&value) {
  return std::make_unique<shared_model::proto::TransactionResponse>(value);
}
