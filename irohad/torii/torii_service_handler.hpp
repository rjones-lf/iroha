/*
Copyright 2017 Soramitsu Co., Ltd.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#ifndef TORII_COMMAND_SERVICE_HANDLER_HPP
#define TORII_COMMAND_SERVICE_HANDLER_HPP

#include <endpoint.grpc.pb.h>
#include <endpoint.pb.h>
#include <network/grpc_async_service.hpp>
#include <network/grpc_call.hpp>
#include "torii/command_service.hpp"
#include "torii/query_service.hpp"

namespace torii {
  /**
   * to handle rpcs loop of CommandService and QueryService.
   */
  class ToriiServiceHandler : public network::GrpcAsyncService {
   public:
    /**
     * requires builder to use same server.
     * @param builder
     */
    ToriiServiceHandler(::grpc::ServerBuilder& builder);

    void assignCommandHandler(
        std::unique_ptr<torii::CommandService> command_service);
    void assignQueryHandler(
        std::unique_ptr<torii::QueryService> query_service);

    virtual ~ToriiServiceHandler() override;

    template <typename RequestType, typename ResponseType>
    using CommandServiceCall =
        network::Call<ToriiServiceHandler,
                      iroha::protocol::CommandService::AsyncService,
                      RequestType, ResponseType>;

    template <typename RequestType, typename ResponseType>
    using QueryServiceCall =
        network::Call<ToriiServiceHandler,
                      iroha::protocol::QueryService::AsyncService, RequestType,
                      ResponseType>;

    /**
     * handles rpcs loop in CommandService.
     */
    virtual void handleRpcs() override;

    /**
     * releases the completion queue of CommandService.
     * @note Call this method after calling server->Shutdown() in ServerRunner
     */
    virtual void shutdown() override;

    /**
     * @return true if completion queue has been shut down.
     */
    bool isShutdownCompletionQueue() const {
      return isShutdownCompletionQueue_;
    }

   private:
    /**
     * helper to call Call::enqueueRequest()
     * @param requester  - pointer to request method. e.g.
     * &CommandService::AsyncService::RequestTorii
     * @param rpcHandler - handler of rpc in ServiceHandler.
     */
    template <typename AsyncService, typename RequestType,
              typename ResponseType>
    void enqueueRequest(
        network::RequestMethod<AsyncService, RequestType, ResponseType>
            requester,
        network::RpcHandler<ToriiServiceHandler, AsyncService, RequestType,
                            ResponseType>
            rpcHandler,
        AsyncService& asyncService) {
      std::unique_lock<std::mutex> lock(mtx_);
      if (!isShutdown_) {
        network::Call<ToriiServiceHandler, AsyncService, RequestType,
                      ResponseType>::enqueueRequest(&asyncService,
                                                    completionQueue_.get(),
                                                    requester, rpcHandler);
      }
    }

    /**
     * extracts request and response from Call instance
     * and calls an actual CommandService::AsyncTorii() implementation.
     * then, creates a new Call instance to serve an another client.
     */
    void ToriiHandler(CommandServiceCall<iroha::protocol::Transaction,
                                         google::protobuf::Empty>*);

    void StatusHandler(CommandServiceCall<iroha::protocol::TxStatusRequest,
                                         iroha::protocol::ToriiResponse>*);


    void QueryFindHandler(QueryServiceCall<iroha::protocol::Query,
                                           iroha::protocol::QueryResponse>*);

   private:
    iroha::protocol::CommandService::AsyncService commandAsyncService_;
    iroha::protocol::QueryService::AsyncService queryAsyncService_;
    std::unique_ptr<grpc::ServerCompletionQueue> completionQueue_;
    std::mutex mtx_;
    bool isShutdown_ = false;                 // called shutdown()
    bool isShutdownCompletionQueue_ = false;  // called cq_->Shutdown()

    std::unique_ptr<torii::CommandService> command_service_;
    std::unique_ptr<torii::QueryService> query_service_;
  };
}  // namespace torii

#endif  // TORII_COMMAND_SERVICE_HANDLER_HPP
