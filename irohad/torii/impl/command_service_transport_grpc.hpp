/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef TORII_COMMAND_SERVICE_TRANSPORT_GRPC_HPP
#define TORII_COMMAND_SERVICE_TRANSPORT_GRPC_HPP

#include "torii/command_service.hpp"

#include <mutex>

#include "endpoint.grpc.pb.h"
#include "endpoint.pb.h"
#include "interfaces/common_objects/transaction_sequence_common.hpp"
#include "interfaces/iroha_internal/abstract_transport_factory.hpp"
#include "logger/logger.hpp"

namespace iroha {
  namespace torii {
    class StatusBus;
  }
}  // namespace iroha

namespace shared_model {
  namespace interface {
    class TxStatusFactory;
    class TransactionBatchParser;
    class TransactionBatchFactory;
  }  // namespace interface
}  // namespace shared_model

namespace iroha {
  namespace network {
    class ConsensusGate;
  }
}  // namespace iroha

namespace torii {
  class CommandServiceTransportGrpc
      : public iroha::protocol::CommandService_v1::Service {
   public:
    using TransportFactoryType =
        shared_model::interface::AbstractTransportFactory<
            shared_model::interface::Transaction,
            iroha::protocol::Transaction>;

    /**
     * Creates a new instance of CommandServiceTransportGrpc
     * @param command_service - to delegate logic work
     * @param status_bus is a common notifier for tx statuses
     * @param status_factory to create stateless invalid tx statuses
     * @param batch_parser to parse the received batches
     * @param transaction_batch_factory to create model batch objects
     * @param consensus_gate to subscribe for rounds updated
     * @param maximum_rounds_without_update - if number of consecutive rounds
     * without status update for any tx becomes higher than this value,
     * streaming breaks off
     */
    CommandServiceTransportGrpc(
        std::shared_ptr<CommandService> command_service,
        std::shared_ptr<iroha::torii::StatusBus> status_bus,
        std::shared_ptr<shared_model::interface::TxStatusFactory>
            status_factory,
        std::shared_ptr<TransportFactoryType> transaction_factory,
        std::shared_ptr<shared_model::interface::TransactionBatchParser>
            batch_parser,
        std::shared_ptr<shared_model::interface::TransactionBatchFactory>
            transaction_batch_factory,
        std::shared_ptr<iroha::network::ConsensusGate> consensus_gate,
        int maximum_rounds_without_update);

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
    /**
     * Flat map transport transactions to shared model
     */
    shared_model::interface::types::SharedTxsCollectionType
    deserializeTransactions(const iroha::protocol::TxList *request);

    std::shared_ptr<CommandService> command_service_;
    std::shared_ptr<iroha::torii::StatusBus> status_bus_;
    std::shared_ptr<shared_model::interface::TxStatusFactory> status_factory_;
    std::shared_ptr<TransportFactoryType> transaction_factory_;
    std::shared_ptr<shared_model::interface::TransactionBatchParser>
        batch_parser_;
    std::shared_ptr<shared_model::interface::TransactionBatchFactory>
        batch_factory_;
    logger::Logger log_;

    std::mutex round_update_mutex_;
    std::shared_ptr<iroha::network::ConsensusGate> consensus_gate_;
    const int kMaximumRoundsWithoutUpdate;
  };
}  // namespace torii

#endif  // TORII_COMMAND_SERVICE_TRANSPORT_GRPC_HPP
