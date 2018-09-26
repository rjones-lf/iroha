/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef TORII_COMMAND_SERVICE_TRANSPORT_GRPC_HPP
#define TORII_COMMAND_SERVICE_TRANSPORT_GRPC_HPP

#include "torii/command_service.hpp"

#include <chrono>

#include "endpoint.grpc.pb.h"
#include "endpoint.pb.h"
#include "interfaces/iroha_internal/tx_status_factory.hpp"
#include "logger/logger.hpp"
#include "torii/status_bus.hpp"

namespace torii {
  class CommandServiceTransportGrpc
      : public iroha::protocol::CommandService::Service {
   public:
    /**
     * Creates a new instance of CommandServiceTransportGrpc
     * @param command_service - to delegate logic work
     * @param status_bus is a common notifier for tx statuses
     * @param initial_timeout - streaming timeout when tx is not received
     * @param nonfinal_timeout - streaming timeout when tx is being processed
     */
    CommandServiceTransportGrpc(
        std::shared_ptr<CommandService> command_service,
        std::shared_ptr<iroha::torii::StatusBus> status_bus,
        std::chrono::milliseconds initial_timeout,
        std::chrono::milliseconds nonfinal_timeout,
        std::shared_ptr<shared_model::interface::TxStatusFactory>
            status_factory);

    /**
     * Torii call via grpc
     * @param context - call context (see grpc docs for details)
     * @param request - transaction received
     * @param response - no actual response (grpc stub for empty answer)
     * @return status
     */
    grpc::Status Torii(grpc::ServerContext *context,
                       const iroha::protocol::Transaction *request,
                       google::protobuf::Empty *response) override;

    /**
     * Torii call for transactions list via grpc
     * @param context - call context (see grpc docs for details)
     * @param request - list of transactions received
     * @param response - no actual response (grpc stub for empty answer)
     * @return status
     */
    grpc::Status ListTorii(grpc::ServerContext *context,
                           const iroha::protocol::TxList *request,
                           google::protobuf::Empty *response) override;

    /**
     * Status call via grpc
     * @param context - call context
     * @param request - TxStatusRequest object which identifies transaction
     * uniquely
     * @param response - ToriiResponse which contains a current state of
     * requested transaction
     * @return status
     */
    grpc::Status Status(grpc::ServerContext *context,
                        const iroha::protocol::TxStatusRequest *request,
                        iroha::protocol::ToriiResponse *response) override;

    /**
     * StatusStream call via grpc
     * @param context - call context
     * @param request - TxStatusRequest object which identifies transaction
     * uniquely
     * @param response_writer - grpc::ServerWriter which can repeatedly send
     * transaction statuses back to the client
     * @return status
     */
    grpc::Status StatusStream(grpc::ServerContext *context,
                              const iroha::protocol::TxStatusRequest *request,
                              grpc::ServerWriter<iroha::protocol::ToriiResponse>
                                  *response_writer) override;

   private:
    std::shared_ptr<CommandService> command_service_;
    std::shared_ptr<iroha::torii::StatusBus> status_bus_;
    const std::chrono::milliseconds initial_timeout_;
    const std::chrono::milliseconds nonfinal_timeout_;
    std::shared_ptr<shared_model::interface::TxStatusFactory> status_factory_;
    logger::Logger log_;
  };
}  // namespace torii

#endif  // TORII_COMMAND_SERVICE_TRANSPORT_GRPC_HPP
