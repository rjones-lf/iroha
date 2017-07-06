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

#include "service.hpp"

namespace ordering {
  namespace connection {

    using iroha::protocol::QueueTransactionResponse;
    using iroha::protocol::Transaction;

    std::function<void(const Transaction&)> dispatchToOrdering;

    void receive(std::function<void(const Transaction&)> const& func) {
      dispatchToOrdering = func;
    }

    grpc::Status OrderingService::QueueTransaction(
        grpc::ClientContext* context,
        const iroha::protocol::Transaction& request,
        QueueTransactionResponse* response) {
      dispatchToOrdering(request);

      return grpc::Status::OK;
    }

  }  // namespace connection
}  // namespace ordering
