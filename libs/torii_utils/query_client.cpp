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

#include <torii_utils/query_client.hpp>
#include <torii/torii_service_handler.hpp>
#include <network/grpc_call.hpp>
#include <block.pb.h>
#include <grpc++/grpc++.h>
#include <thread>

namespace torii_utils {
  
  using iroha::protocol::Query;
  using iroha::protocol::QueryResponse;

  QuerySyncClient::QuerySyncClient(const std::string& ip, const int port)
    : stub_(iroha::protocol::QueryService::NewStub(
    grpc::CreateChannel(ip + ":" + std::to_string(port), grpc::InsecureChannelCredentials())))
  {}

  QuerySyncClient::~QuerySyncClient() {
    completionQueue_.Shutdown();
  }

  /**
   * requests query to a torii server and returns response (blocking, sync)
   * @param query
   * @param response
   * @return grpc::Status
   */
  grpc::Status QuerySyncClient::Find(const iroha::protocol::Query &query, QueryResponse &response) {

    std::unique_ptr<grpc::ClientAsyncResponseReader<iroha::protocol::QueryResponse>> rpc(
      stub_->AsyncFind(&context_, query, &completionQueue_)
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

}  // namespace torii
