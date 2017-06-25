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
#include <grpc++/server_context.h>
#include <ordering/queue.hpp>
#include <logger/logger.hpp>
#include <validation/stateless/validator.hpp>

static logger::Logger Log("CommandService");

namespace api {

  using namespace iroha::protocol;

  std::function<void(const Transaction&)> dispatchToOrdering;

  /**
   * Sets function to dispatch tx to ordering queue.
   * @param func
   */
  void receive(
      std::function<void(const iroha::protocol::Transaction&)> const& func) {
    dispatchToOrdering = func;
  }

  /**
   * Receives tx from clients then pass it to the ordering service's server.
   * @param context - used by getting client's ip and port.
   * @param tx - tx to be sent.
   * @param response - used for replying tx status (failed with connection,
   *                   stateless validation / successfully send tx)
   * @return grpc::Status
   */
  grpc::Status CommandService::Torii(grpc::ServerContext* context,
                                     const Transaction* tx,
                                     ToriiResponse* response) {
    // TODO: Use this to get client's ip and port.
    (void)context;

    if (validation::stateless::validate(*tx)) {
      // TODO: Return tracking log number (hash)
      try {
        dispatchToOrdering(*tx);
        response->set_code(ResponseCode::OK);
        response->set_message("successfully dispatching tx to ordering.");
      } catch(...) {
        Log.error("ordering::::initialize()");
      }
    } else {
      // TODO: Return validation failed message
      response->set_code(ResponseCode::FAIL);
      response->set_message("failed stateless validation.");
    }
    return grpc::Status::OK;
  }

}  // namespace api
