/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_MST_TRANSPORT_STUB_HPP
#define IROHA_MST_TRANSPORT_STUB_HPP

#include "mst.grpc.pb.h"
#include "network/mst_transport.hpp"

#include <google/protobuf/empty.pb.h>
#include "interfaces/common_objects/common_objects_factory.hpp"
#include "interfaces/iroha_internal/abstract_transport_factory.hpp"
#include "interfaces/iroha_internal/transaction_batch_factory.hpp"
#include "interfaces/iroha_internal/transaction_batch_parser.hpp"
#include "logger/logger.hpp"
#include "network/impl/async_grpc_client.hpp"

namespace iroha {
  namespace network {
    class MstTransportStub : public MstTransportGrpc {
     public:
      using MstTransportGrpc::MstTransportGrpc;

      void sendState(const shared_model::interface::Peer &to,
                     ConstRefState providing_state) override {}
    };
  }  // namespace network
}  // namespace iroha

#endif  // IROHA_MST_TRANSPORT_STUB_HPP
