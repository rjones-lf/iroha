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

#include <torii/command_client.hpp>
#include <torii/torii_service_handler.hpp>
#include <network/grpc_call.hpp>
#include <block.pb.h>
#include <grpc++/grpc++.h>
#include <thread>

namespace torii {

  using iroha::protocol::Transaction;
  using iroha::protocol::ToriiResponse;

  CommandSyncClient::CommandSyncClient(const std::string& ip, const int port)
    : stub_(iroha::protocol::CommandService::NewStub(
    grpc::CreateChannel(ip + ":" + std::to_string(port), grpc::InsecureChannelCredentials())))
  {}

  CommandSyncClient::~CommandSyncClient() {
    completionQueue_.Shutdown();
  }

  /**
   * requests tx to a torii server and returns response (blocking, sync)
   * @param tx
   * @param response - returns ToriiResponse if succeeded
   * @return grpc::Status - returns connection is success or not.
   */
  grpc::Status CommandSyncClient::Torii(const Transaction& tx, ToriiResponse& response) {

    std::unique_ptr<grpc::ClientAsyncResponseReader<iroha::protocol::ToriiResponse>> rpc(
      stub_->AsyncTorii(&context_, tx, &completionQueue_)
    );

    using State = network::UntypedCall<torii::ToriiServiceHandler>::State;

    rpc->Finish(&response, &status_, (void *)static_cast<int>(State::ResponseSent));

    void* got_tag;
    bool ok = false;

    /**
     * pulls a new rpc response. If no response, blocks this thread.
     */
    if (!completionQueue_.Next(&got_tag, &ok)) {
      throw std::runtime_error("CompletionQueue::Next() returns error");
    }

    assert(got_tag == (void *)static_cast<int>(State::ResponseSent));
    assert(ok);

    return status_;
  }

  /**
   * manages state of a Torii async client call.
   */
  struct ToriiAsyncClientCall {
    iroha::protocol::ToriiResponse response;
    grpc::ClientContext context;
    grpc::Status status;
    std::unique_ptr<grpc::ClientAsyncResponseReader<iroha::protocol::ToriiResponse>> responseReader;
    CommandAsyncClient::Callback callback;
  };

  /**
   * requests tx to a torii server and returns response (non-blocking)
   * @param tx
   * @param callback
   * @return grpc::Status
   */
  grpc::Status CommandAsyncClient::Torii(
    const Transaction& tx,
    const std::function<void(ToriiResponse& response)>& callback)
  {
    auto call = new ToriiAsyncClientCall;
    call->callback = callback;
    call->responseReader = stub_->AsyncTorii(&call->context, tx, &completionQueue_);
    call->responseReader->Finish(&call->response, &call->status, (void*)call);
    return call->status;
  }

  /**
   * sets ip and port and calls listenToriiNonBlocking() in a new thread.
   * @param ip
   * @param port
   */
  CommandAsyncClient::CommandAsyncClient(const std::string& ip, const int port)
    : stub_(iroha::protocol::CommandService::NewStub(
    grpc::CreateChannel(ip + ":" + std::to_string(port), grpc::InsecureChannelCredentials())))
  {
    listener_ = std::thread(&CommandAsyncClient::listen, this);
  }

  CommandAsyncClient::~CommandAsyncClient() {
    completionQueue_.Shutdown();
    listener_.join();
  }

  /**
   * starts response listener of a non-blocking torii client.
   */
  void CommandAsyncClient::listen() {

    /**
     * got_tag - a state (ToriiAsyncClientCall) that is sent by a command server.
     * ok - true if a regular event, otherwise false (e.g. grpc::Alarm)
     */
    void* got_tag;
    bool ok = false;

    /**
     * pulls a new client's response. If no response, blocks this thread.
     * CompletionQueue::Next() returns false if cq_.Shutdown() is executed.
     */
    while (completionQueue_.Next(&got_tag, &ok)) {

      if (!got_tag || !ok) {
        break;
      }

      auto call = static_cast<ToriiAsyncClientCall*>(got_tag);

      if (call->status.ok()) {
        call->callback(call->response);
      } else {
        ToriiResponse responseFailure;
        responseFailure.set_code(iroha::protocol::ResponseCode::FAIL);
        responseFailure.set_message("RPC failed");
        call->callback(responseFailure);
      }

      delete call;
    }
  }

}  // namespace torii
