/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "backend/protobuf/proto_tx_status_factory.hpp"

#include "interfaces/common_objects/types.hpp"

using namespace shared_model::proto;

// -------------------------------| Private API |-------------------------------

namespace {
  /**
   * Fills common fields for all statuses
   */
  iroha::protocol::ToriiResponse fillCommon(
      ProtoTxStatusFactory::TransactionHashType hash,
      ProtoTxStatusFactory::StatelessErrorOrFailedCommandNameType error_or_cmd,
      ProtoTxStatusFactory::FailedCommandIndexType index,
      ProtoTxStatusFactory::ErrorCodeType error_code,
      iroha::protocol::TxStatus status) {
    iroha::protocol::ToriiResponse response;
    response.set_tx_hash(shared_model::crypto::toBinaryString(hash));
    response.set_err_or_cmd_name(error_or_cmd);
    response.set_failed_cmd_index(index);
    response.set_error_code(error_code);
    response.set_tx_status(status);
    return response;
  }

  /**
   * Wraps status with model object
   */
  ProtoTxStatusFactory::FactoryReturnType wrap(
      iroha::protocol::ToriiResponse &&value) {
    return std::make_unique<shared_model::proto::TransactionResponse>(
        std::move(value));
  }
}  // namespace

// ---------------------------| Stateless statuses |----------------------------

ProtoTxStatusFactory::FactoryReturnType ProtoTxStatusFactory::makeStatelessFail(
    TransactionHashType hash,
    StatelessErrorOrFailedCommandNameType error_or_cmd,
    FailedCommandIndexType index,
    ErrorCodeType error_code) {
  return wrap(
      fillCommon(hash,
                 error_or_cmd,
                 index,
                 error_code,
                 iroha::protocol::TxStatus::STATELESS_VALIDATION_FAILED));
}

ProtoTxStatusFactory::FactoryReturnType
ProtoTxStatusFactory::makeStatelessValid(
    TransactionHashType hash,
    StatelessErrorOrFailedCommandNameType error_or_cmd,
    FailedCommandIndexType index,
    ErrorCodeType error_code) {
  return wrap(
      fillCommon(hash,
                 error_or_cmd,
                 index,
                 error_code,
                 iroha::protocol::TxStatus::STATELESS_VALIDATION_SUCCESS));
}

// ---------------------------| Stateful statuses |-----------------------------

ProtoTxStatusFactory::FactoryReturnType ProtoTxStatusFactory::makeStatefulFail(
    TransactionHashType hash,
    StatelessErrorOrFailedCommandNameType error_or_cmd,
    FailedCommandIndexType index,
    ErrorCodeType error_code) {
  return wrap(
      fillCommon(hash,
                 error_or_cmd,
                 index,
                 error_code,
                 iroha::protocol::TxStatus::STATEFUL_VALIDATION_FAILED));
}
ProtoTxStatusFactory::FactoryReturnType ProtoTxStatusFactory::makeStatefulValid(
    TransactionHashType hash,
    StatelessErrorOrFailedCommandNameType error_or_cmd,
    FailedCommandIndexType index,
    ErrorCodeType error_code) {
  return wrap(
      fillCommon(hash,
                 error_or_cmd,
                 index,
                 error_code,
                 iroha::protocol::TxStatus::STATEFUL_VALIDATION_SUCCESS));
}

// -----------------------------| Final statuses |------------------------------

ProtoTxStatusFactory::FactoryReturnType ProtoTxStatusFactory::makeCommitted(
    TransactionHashType hash,
    StatelessErrorOrFailedCommandNameType error_or_cmd,
    FailedCommandIndexType index,
    ErrorCodeType error_code) {
  return wrap(fillCommon(hash,
                         error_or_cmd,
                         index,
                         error_code,
                         iroha::protocol::TxStatus::COMMITTED));
}

ProtoTxStatusFactory::FactoryReturnType ProtoTxStatusFactory::makeRejected(
    TransactionHashType hash,
    StatelessErrorOrFailedCommandNameType error_or_cmd,
    FailedCommandIndexType index,
    ErrorCodeType error_code) {
  return wrap(fillCommon(hash,
                         error_or_cmd,
                         index,
                         error_code,
                         iroha::protocol::TxStatus::REJECTED));
}

// -----------------------------| Rest statuses |-------------------------------

ProtoTxStatusFactory::FactoryReturnType ProtoTxStatusFactory::makeMstExpired(
    TransactionHashType hash,
    StatelessErrorOrFailedCommandNameType error_or_cmd,
    FailedCommandIndexType index,
    ErrorCodeType error_code) {
  return wrap(fillCommon(hash,
                         error_or_cmd,
                         index,
                         error_code,
                         iroha::protocol::TxStatus::MST_EXPIRED));
}

ProtoTxStatusFactory::FactoryReturnType ProtoTxStatusFactory::makeNotReceived(
    TransactionHashType hash,
    StatelessErrorOrFailedCommandNameType error_or_cmd,
    FailedCommandIndexType index,
    ErrorCodeType error_code) {
  return wrap(fillCommon(hash,
                         error_or_cmd,
                         index,
                         error_code,
                         iroha::protocol::TxStatus::NOT_RECEIVED));
}

ProtoTxStatusFactory::FactoryReturnType
ProtoTxStatusFactory::makeEnoughSignaturesCollected(
    TransactionHashType hash,
    StatelessErrorOrFailedCommandNameType error_or_cmd,
    FailedCommandIndexType index,
    ErrorCodeType error_code) {
  return wrap(
      fillCommon(hash,
                 error_or_cmd,
                 index,
                 error_code,
                 iroha::protocol::TxStatus::ENOUGH_SIGNATURES_COLLECTED));
}
