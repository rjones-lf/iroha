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

#ifndef IROHA_PROTO_TRANSACTION_STATUS_BUILDER_HPP
#define IROHA_PROTO_TRANSACTION_STATUS_BUILDER_HPP

#include "backend/protobuf/transaction_responses/proto_tx_response.hpp"

namespace shared_model {
  namespace proto {
    class TransactionStatusBuilder {
     public:
      shared_model::proto::TransactionResponse build() &&;

      shared_model::proto::TransactionResponse build() &;

      TransactionStatusBuilder statelessValidationSuccess();

      TransactionStatusBuilder statelessValidationFailed();

      TransactionStatusBuilder statefulValidationSuccess();

      TransactionStatusBuilder statefulValidationFailed();

      TransactionStatusBuilder committed();

      TransactionStatusBuilder notReceived();

      TransactionStatusBuilder txHash(const crypto::Hash& hash);

     private:
      iroha::protocol::ToriiResponse tx_response_;
    };
  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_PROTO_TRANSACTION_STATUS_BUILDER_HPP
