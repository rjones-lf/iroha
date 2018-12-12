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

#include "torii/query_client.hpp"

#include "network/impl/grpc_channel_builder.hpp"

namespace torii_utils {

  using iroha::protocol::Query;
  using iroha::protocol::QueryResponse;

  QuerySyncClient::QuerySyncClient(const std::string &ip, size_t port)
      : ip_(ip),
        port_(port),
        stub_(iroha::network::createClient<iroha::protocol::QueryService_v1>(
            ip + ":" + std::to_string(port))) {}

  QuerySyncClient::QuerySyncClient(const QuerySyncClient &rhs)
      : QuerySyncClient(rhs.ip_, rhs.port_) {}

  QuerySyncClient &QuerySyncClient::operator=(QuerySyncClient rhs) {
    swap(*this, rhs);
    return *this;
  }

  QuerySyncClient::QuerySyncClient(QuerySyncClient &&rhs) noexcept {
    swap(*this, rhs);
  }

  QuerySyncClient &QuerySyncClient::operator=(QuerySyncClient &&rhs) noexcept {
    swap(*this, rhs);
    return *this;
  }

  /**
   * requests query to a torii server and returns response (blocking, sync)
   * @param query
   * @param response
   * @return grpc::Status
   */
  grpc::Status QuerySyncClient::Find(const iroha::protocol::Query &query,
                                     QueryResponse &response) const {
    grpc::ClientContext context;
    return stub_->Find(&context, query, &response);
  }

  std::vector<iroha::protocol::BlockQueryResponse>
  QuerySyncClient::FetchCommits(
      const iroha::protocol::BlocksQuery &blocks_query) const {
    grpc::ClientContext context;
    auto reader = stub_->FetchCommits(&context, blocks_query);
    std::vector<iroha::protocol::BlockQueryResponse> responses;
    iroha::protocol::BlockQueryResponse resp;
    while (reader->Read(&resp)) {
      responses.push_back(resp);
    }
    reader->Finish();
    return responses;
  }

  void QuerySyncClient::swap(QuerySyncClient &lhs, QuerySyncClient &rhs) {
    using std::swap;
    swap(lhs.ip_, rhs.ip_);
    swap(lhs.port_, rhs.port_);
    swap(lhs.stub_, rhs.stub_);
  }

}  // namespace torii_utils
