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

#ifndef TORII_COMMAND_CLIENT_HPP
#define TORII_COMMAND_CLIENT_HPP

#include <endpoint.grpc.pb.h>
#include <grpc++/grpc++.h>
#include <memory>
#include <thread>

namespace torii {

  /**
   * CommandSyncClient
   */
  class CommandSyncClient {
   public:
    CommandSyncClient(std::string ip, int port);
    ~CommandSyncClient();

    /**
     * requests tx to a torii server and returns response (blocking, sync)
     * @param tx
     * @return grpc::Status - returns connection is success or not.
     */
    grpc::Status Torii(const iroha::protocol::Transaction& tx);

    /**
     * @param tx
     * @param response returns ToriiResponse if succeeded
     * @return grpc::Status - returns connection is success or not.
     */
    grpc::Status Status(const iroha::protocol::TxStatusRequest& tx,
                        iroha::protocol::ToriiResponse& response);

   private:
    grpc::ClientContext context_;
    std::unique_ptr<iroha::protocol::CommandService::Stub> stub_;
    grpc::CompletionQueue completionQueue_;
    grpc::Status status_;
  };

  /**
   * CommandAsyncClient is used by peer service.
   */
  class CommandAsyncClient {
   public:
    /**
     * sets ip and port and calls listenToriiNonBlocking() in a new thread.
     * @param ip
     * @param port
     */
    CommandAsyncClient(const std::string& ip, const int port);

    ~CommandAsyncClient();

    using ToriiCallback = std::function<void(google::protobuf::Empty& response)>;
    using StatusCallback = std::function<void(iroha::protocol::ToriiResponse)>;

    /**
     * Async Torii rpc
     * @param tx
     * @param callback
     * @return grpc::Status
     */
    grpc::Status Torii(const iroha::protocol::Transaction& tx,
                       const ToriiCallback& callback);

    /**
     * @param tx_request contains hash of requested tx
     * @param callback processes obtained response
     * @return grpc::Status
     */
    grpc::Status Status(const iroha::protocol::TxStatusRequest& tx_request,
                        const StatusCallback& callback);

   private:
    /**
     * starts response listener of non-blocking rpcs.
     */
    void listen();

   private:
    grpc::ClientContext context_;
    std::unique_ptr<iroha::protocol::CommandService::Stub> stub_;
    grpc::CompletionQueue completionQueue_;
    grpc::Status status_;
    std::thread listener_;  // listens rpcs' responses and executes callbacks.
    std::thread status_listener_;
  };

}  // namespace torii

#endif  // TORII_COMMAND_CLIENT_HPP
