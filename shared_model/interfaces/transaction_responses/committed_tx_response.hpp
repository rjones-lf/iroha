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

#ifndef IROHA_COMMITTED_TX_RESPONSE_HPP
#define IROHA_COMMITTED_TX_RESPONSE_HPP

#include "interfaces/transaction_responses/abstract_tx_response.hpp"

namespace shared_model {
  namespace interface {
    /**
     * Tx pipeline succeeded, tx is committed in ledger
     */
    class CommittedTxResponse : public AbstractTxResponse<CommittedTxResponse> {
     private:
      std::string className() const override {
        return "CommittedTxResponse";
      }
    };

  }  // namespace interface
}  // namespace shared_model
#endif  // IROHA_COMMITTED_TX_RESPONSE_HPP
