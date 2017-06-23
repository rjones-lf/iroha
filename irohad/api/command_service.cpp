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

#include "command_service.hpp"
#include <ordering/queue.hpp>
#include <validation/stateless/validator.hpp>

namespace connection {
  namespace api {

    using namespace iroha::protocol;

    static auto log = logger::Logger("commandService");


    std::function<void(const Transaction&)> dispatchToOrdering;

    void receive(
        std::function<void(const iroha::protocol::Transaction&)> const& func) {
      dispatchToOrdering = func;
    }

    grpc::Status CommandService::Torii(grpc::ServerContext* context,
                                       const ::iroha::protocol::Transaction* request,
                                       ::iroha::protocol::ToriiResponse* response){
      // TODO: Use this to get client's ip and port.
      (void)context;
      log.debug("Torii: receive Transaction");

      if (validator::stateless::validate(*request)) {
        log.debug("Torii: transaction is valid");
        dispatchToOrdering(*request);
        // TODO: Return tracking log number (hash)
        *response = ToriiResponse();
      } else {
        log.warning("Torii: transaction is invalid");
        // TODO: Return validation failed message
        *response = ToriiResponse();
      }
      return grpc::Status::OK;
    }

  }  // namespace api
}  // namespace connection
