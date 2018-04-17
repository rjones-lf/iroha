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

#ifndef IROHA_ASYNC_GRPC_CLIENT_HPP
#define IROHA_ASYNC_GRPC_CLIENT_HPP

#include <google/protobuf/empty.pb.h>
#include <grpc++/grpc++.h>
#include <thread>

namespace iroha {
  namespace network {

    /**
     * Asynchronous gRPC client which does no processing of server responses
     * @tparam Response type of server response
     */
    template <typename Response>
    class AsyncGrpcClient {
     public:
      explicit AsyncGrpcClient(logger::Logger &&log)
          : thread_(&AsyncGrpcClient::asyncCompleteRpc, this),
            log_(std::move(log)) {}

      /**
       * Listen to gRPC server responses
       */
      void asyncCompleteRpc() {
        void *got_tag;
        auto ok = false;
        while (cq_.Next(&got_tag, &ok)) {
          auto call = static_cast<AsyncClientCall *>(got_tag);
          if (not call->status.ok()) {
            log_->warn("RPC failed: {}", call->status.error_message());
          }
          delete call;
        }
      }

      ~AsyncGrpcClient() {
        cq_.Shutdown();
        if (thread_.joinable()) {
          thread_.join();
        }
      }

      grpc::CompletionQueue cq_;
      std::thread thread_;
      logger::Logger log_;

      /**
       * State and data information of gRPC call
       */
      struct AsyncClientCall {
        Response reply;

        grpc::ClientContext context;

        grpc::Status status;

        std::unique_ptr<grpc::ClientAsyncResponseReader<Response>>
            response_reader;
      };
    };
  }  // namespace network
}  // namespace iroha

#endif  // IROHA_ASYNC_GRPC_CLIENT_HPP
