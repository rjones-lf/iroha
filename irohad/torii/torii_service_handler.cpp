/*
Copyright 2016 Soramitsu Co., Ltd.

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

#include <endpoint.grpc.pb.h>
#include <google/protobuf/empty.pb.h>
#include <grpc/support/time.h>
#include <unistd.h>
#include <network/grpc_async_service.hpp>
#include <network/grpc_call.hpp>
#include <torii/command_service.hpp>
#include <torii/query_service.hpp>
#include <torii/torii_service_handler.hpp>

namespace prot = iroha::protocol;

namespace torii {

  /**
   * registers async command service
   * @param builder
   */
  ToriiServiceHandler::ToriiServiceHandler(::grpc::ServerBuilder& builder) {
    builder.RegisterService(&commandAsyncService_);
    builder.RegisterService(&queryAsyncService_);
    completionQueue_ = builder.AddCompletionQueue();
  }

  ToriiServiceHandler::~ToriiServiceHandler() {}

  /**
   * shuts down service handler. (actually, shuts down completion queue only)
   */
  void ToriiServiceHandler::shutdown() { completionQueue_->Shutdown(); }

  /**
   * handles rpcs loop in CommandService.
   */
  void ToriiServiceHandler::handleRpcs() {
    // CommandService::Torii()
    enqueueRequest<prot::CommandService::AsyncService, prot::Transaction,
                   google::protobuf::Empty>(
        &prot::CommandService::AsyncService::RequestTorii,
        &ToriiServiceHandler::ToriiHandler, commandAsyncService_);

    enqueueRequest<prot::CommandService::AsyncService, prot::TxStatusRequest,
                   prot::ToriiResponse>(
        &prot::CommandService::AsyncService::RequestStatus,
        &ToriiServiceHandler::StatusHandler, commandAsyncService_);

    // QueryService::Find()
    enqueueRequest<prot::QueryService::AsyncService, prot::Query,
                   prot::QueryResponse>(
        &prot::QueryService::AsyncService::RequestFind,
        &ToriiServiceHandler::QueryFindHandler, queryAsyncService_);

    /**
     * tag is a state corresponding to one rpc connection.
     * ok is true if read a regular event, false otherwise (e.g. grpc::Alarm is
     * not a regular event).
     */
    void* tag;
    bool ok;

    /**
     * pulls a state of a new client's rpc request from completion queue.
     * If no request, CompletionQueue::Next() waits a new request (blocks this
     * thread). CompletionQueue::Next() returns false if
     * completionQueue_->Shutdown() is executed.
     */
    while (completionQueue_->Next(&tag, &ok)) {
      auto callbackTag =
          static_cast<network::UntypedCall<ToriiServiceHandler>::CallOwner*>(
              tag);
      if (ok && callbackTag) {
        /*assert(callbackTag);*/
        callbackTag->onCompleted(this);
      } else {
        isShutdownCompletionQueue_ = true;
        break;
      }
    }
  }

  /**
   * extracts request and response from Call instance
   * and calls an actual CommandService::AsyncTorii() implementation.
   * then, spawns a new Call instance to serve an another client.
   */
  void ToriiServiceHandler::ToriiHandler(
      CommandServiceCall<prot::Transaction, google::protobuf::Empty>* call) {
    command_service_->ToriiAsync(call->request(), call->response());
    call->sendResponse(grpc::Status::OK);

    // Spawn a new Call instance to serve an another client.
    enqueueRequest<prot::CommandService::AsyncService, prot::Transaction,
                   google::protobuf::Empty>(
        &prot::CommandService::AsyncService::RequestTorii,
        &ToriiServiceHandler::ToriiHandler, commandAsyncService_);
  }

  void ToriiServiceHandler::StatusHandler(
      CommandServiceCall<iroha::protocol::TxStatusRequest,
                         iroha::protocol::ToriiResponse>* call) {
    command_service_->StatusAsync(call->request(), call->response());
    call->sendResponse(grpc::Status::OK);

    enqueueRequest<prot::CommandService::AsyncService, prot::TxStatusRequest,
                   iroha::protocol::ToriiResponse>(
        &prot::CommandService::AsyncService::RequestStatus,
        &ToriiServiceHandler::StatusHandler, commandAsyncService_);
  }

  void ToriiServiceHandler::QueryFindHandler(
      QueryServiceCall<iroha::protocol::Query, iroha::protocol::QueryResponse>*
          call) {
    query_service_->FindAsync(call->request(), call->response());
    call->sendResponse(grpc::Status::OK);

    // Spawn a new Call instance to serve an another client.
    enqueueRequest<prot::QueryService::AsyncService, prot::Query,
                   prot::QueryResponse>(
        &prot::QueryService::AsyncService::RequestFind,
        &ToriiServiceHandler::QueryFindHandler, queryAsyncService_);
  }
  void ToriiServiceHandler::assignCommandHandler(
      std::unique_ptr<torii::CommandService> command_service) {
    command_service_ = std::move(command_service);
  }
  void ToriiServiceHandler::assignQueryHandler(
      std::unique_ptr<torii::QueryService> query_service) {
    query_service_ = std::move(query_service);
  }

}  // namespace torii
