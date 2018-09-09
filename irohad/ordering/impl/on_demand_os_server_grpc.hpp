/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_ON_DEMAND_OS_TRANSPORT_SERVER_GRPC_HPP
#define IROHA_ON_DEMAND_OS_TRANSPORT_SERVER_GRPC_HPP

#include "ordering/on_demand_os_transport.hpp"

#include "ordering.grpc.pb.h"

namespace iroha {
  namespace ordering {
    namespace transport {

      /**
       * gRPC server for on demand ordering service
       */
      class OnDemandOsServerGrpc : public proto::OnDemandOrdering::Service {
       public:
        explicit OnDemandOsServerGrpc(
            std::shared_ptr<OdOsNotification> ordering_service);

        grpc::Status SendTransactions(
            ::grpc::ServerContext *context,
            const proto::TransactionsRequest *request,
            ::google::protobuf::Empty *response) override;

        grpc::Status RequestProposal(
            ::grpc::ServerContext *context,
            const proto::ProposalRequest *request,
            proto::ProposalResponse *response) override;

       private:
        std::shared_ptr<OdOsNotification> ordering_service_;
      };

    }  // namespace transport
  }    // namespace ordering
}  // namespace iroha

#endif  // IROHA_ON_DEMAND_OS_TRANSPORT_SERVER_GRPC_HPP
