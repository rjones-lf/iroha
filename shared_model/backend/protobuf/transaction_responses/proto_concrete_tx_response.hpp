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

#include "backend/protobuf/common_objects/trivial_proto.hpp"
#include "endpoint.pb.h"
#include "interfaces/transaction_responses/committed_tx_response.hpp"
#include "interfaces/transaction_responses/not_received_tx_response.hpp"
#include "interfaces/transaction_responses/stateful_failed_tx_response.hpp"
#include "interfaces/transaction_responses/stateful_valid_tx_response.hpp"
#include "interfaces/transaction_responses/stateless_failed_tx_response.hpp"
#include "interfaces/transaction_responses/stateless_valid_tx_response.hpp"
#include "interfaces/transaction_responses/tx_response.hpp"

namespace shared_model {
  namespace proto {
    using StatelessFailedTxResponse =
        TrivialProto<interface::StatelessFailedTxResponse,
                     iroha::protocol::ToriiResponse>;
    using StatelessValidTxResponse =
        TrivialProto<interface::StatelessValidTxResponse,
                     iroha::protocol::ToriiResponse>;
    using StatefulFailedTxResponse =
        TrivialProto<interface::StatefulFailedTxResponse,
                     iroha::protocol::ToriiResponse>;
    using StatefulValidTxResponse =
        TrivialProto<interface::StatefulValidTxResponse,
                     iroha::protocol::ToriiResponse>;
    using CommittedTxResponse = TrivialProto<interface::CommittedTxResponse,
                                             iroha::protocol::ToriiResponse>;
    using NotReceivedTxResponse = TrivialProto<interface::NotReceivedTxResponse,
                                               iroha::protocol::ToriiResponse>;
  }  // namespace proto
}  // namespace shared_model
