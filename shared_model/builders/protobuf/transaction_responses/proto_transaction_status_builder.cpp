/**
 * Copyright Soramitsu Co., Ltd. 2018 All Rights Reserved.
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

#include "builders/protobuf/transaction_responses/proto_transaction_status_builder.hpp"

namespace shared_model {
  namespace proto {

    shared_model::proto::TransactionResponse TransactionStatusBuilder::build()
        && {
      return shared_model::proto::TransactionResponse(std::move(tx_response_));
    }

    shared_model::proto::TransactionResponse TransactionStatusBuilder::build()
        & {
      return shared_model::proto::TransactionResponse(
          iroha::protocol::ToriiResponse(tx_response_));
    }

    TransactionStatusBuilder
    TransactionStatusBuilder::statelessValidationSuccess() {
      TransactionStatusBuilder copy(*this);
      copy.tx_response_.set_tx_status(
          iroha::protocol::TxStatus::STATELESS_VALIDATION_SUCCESS);
      return copy;
    }

    TransactionStatusBuilder
    TransactionStatusBuilder::statelessValidationFailed() {
      TransactionStatusBuilder copy(*this);
      copy.tx_response_.set_tx_status(
          iroha::protocol::TxStatus::STATELESS_VALIDATION_FAILED);
      return copy;
    }

    TransactionStatusBuilder
    TransactionStatusBuilder::statefulValidationSuccess() {
      TransactionStatusBuilder copy(*this);
      copy.tx_response_.set_tx_status(
          iroha::protocol::TxStatus::STATEFUL_VALIDATION_SUCCESS);
      return copy;
    }

    TransactionStatusBuilder
    TransactionStatusBuilder::statefulValidationFailed() {
      TransactionStatusBuilder copy(*this);
      copy.tx_response_.set_tx_status(
          iroha::protocol::TxStatus::STATEFUL_VALIDATION_FAILED);
      return copy;
    }

    TransactionStatusBuilder TransactionStatusBuilder::committed() {
      TransactionStatusBuilder copy(*this);
      copy.tx_response_.set_tx_status(iroha::protocol::TxStatus::COMMITTED);
      return copy;
    }

    TransactionStatusBuilder TransactionStatusBuilder::notReceived() {
      TransactionStatusBuilder copy(*this);
      copy.tx_response_.set_tx_status(iroha::protocol::TxStatus::NOT_RECEIVED);
      return copy;
    }

    TransactionStatusBuilder TransactionStatusBuilder::txHash(
        const crypto::Hash &hash) {
      TransactionStatusBuilder copy(*this);
      copy.tx_response_.set_tx_hash(crypto::toBinaryString(hash));
      return copy;
    }

  }  // namespace proto
}  // namespace shared_model
