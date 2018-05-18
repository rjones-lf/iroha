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

#ifndef IROHA_STATEFUL_VALID_TX_RESPONSE_HPP
#define IROHA_STATEFUL_VALID_TX_RESPONSE_HPP

#include "utils/string_builder.hpp"

namespace shared_model {
  namespace interface {
    /**
     * Tx response of passed stateful validation
     */
    class StatefulValidTxResponse
        : public AbstractTxResponse<StatefulValidTxResponse> {
     private:
      std::string className() const override {
        return "StatefulValidTxResponse";
      }
    };
  }  // namespace interface
}  // namespace shared_model
#endif  // IROHA_STATEFUL_VALID_TX_RESPONSE_HPP
