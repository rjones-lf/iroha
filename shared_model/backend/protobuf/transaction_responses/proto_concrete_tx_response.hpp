/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "backend/protobuf/common_objects/trivial_proto.hpp"
#include "endpoint.pb.h"
#include "interfaces/transaction_responses/committed_tx_response.hpp"
#include "interfaces/transaction_responses/mst_expired_response.hpp"
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
    using MstExpiredResponse = TrivialProto<interface::MstExpiredResponse,
                                            iroha::protocol::ToriiResponse>;
    using NotReceivedTxResponse = TrivialProto<interface::NotReceivedTxResponse,
                                               iroha::protocol::ToriiResponse>;
  }  // namespace proto
}  // namespace shared_model
