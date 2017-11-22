/**
 * Copyright Soramitsu Co., Ltd. 2017 All Rights Reserved.
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

#ifndef IROHA_PROTO_STATELESS_VALID_TX_RESPONSE_HPP
#define IROHA_PROTO_STATELESS_VALID_TX_RESPONSE_HPP

#include "endpoint.pb.h"
#include "interfaces/transaction_responses/stateless_valid_tx_response.hpp"
#include "utils/reference_holder.hpp"

namespace shared_model {
  namespace proto {
    /**
     * Stateless validation passed
     */
    class StatelessValidTxResponse final
        : public interface::StatelessValidTxResponse {
     private:
      using RefStatelessValidTxResponse =
          detail::ReferenceHolder<iroha::protocol::ToriiResponse>;

     public:
      explicit StatelessValidTxResponse(
          const iroha::protocol::ToriiResponse &response)
          : StatelessValidTxResponse(RefStatelessValidTxResponse(response)) {}

      explicit StatelessValidTxResponse(
          iroha::protocol::ToriiResponse &&response)
          : StatelessValidTxResponse(
                RefStatelessValidTxResponse(std::move(response))) {}

      ModelType *copy() const override {
        iroha::protocol::ToriiResponse response;
        response.set_tx_status(iroha::protocol::STATELESS_VALIDATION_SUCCESS);
        return new StatelessValidTxResponse(std::move(response));
      }

     private:
      explicit StatelessValidTxResponse(RefStatelessValidTxResponse &&ref)
          : response_(std::move(ref)) {}
      // proto
      RefStatelessValidTxResponse response_;
    };
  }  // namespace  proto
}  // namespace shared_model
#endif  // IROHA_PROTO_STATELESS_VALID_TX_RESPONSE_HPP
