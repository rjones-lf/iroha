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

#ifndef TORII_COMMAND_SERVICE_HPP
#define TORII_COMMAND_SERVICE_HPP

#include <endpoint.grpc.pb.h>
#include <endpoint.pb.h>
#include <iostream>
#include <string>
#include <unordered_map>
#include "model/converters/pb_transaction_factory.hpp"
#include "model/transaction_response.hpp"
#include "torii/processor/transaction_processor.hpp"

namespace torii {

  /**
   * Actual implementation of async CommandService.
   * ToriiServiceHandler::(SomeMethod)Handler calls a corresponding method in
   * this class.
   */
  class CommandService {
   public:
    CommandService(
        std::shared_ptr<iroha::model::converters::PbTransactionFactory>
            pb_factory,
        std::shared_ptr<iroha::torii::TransactionProcessor> txProccesor);

    CommandService(const CommandService&) = delete;
    CommandService& operator=(const CommandService&) = delete;
    /**
     * actual implementation of async Torii in CommandService
     * @param request - Transaction
     * @param response - ToriiResponse
     */
    void ToriiAsync(iroha::protocol::Transaction const& request,
                    google::protobuf::Empty& response);

    void StatusAsync(iroha::protocol::TxStatusRequest const& request,
                     iroha::protocol::ToriiResponse& response);

   private:
    std::shared_ptr<iroha::model::converters::PbTransactionFactory> pb_factory_;
    std::shared_ptr<iroha::torii::TransactionProcessor> tx_processor_;
    std::unordered_map<std::string, iroha::protocol::ToriiResponse>
        handler_map_;
  };

}  // namespace torii

#endif  // TORII_COMMAND_SERVICE_HPP
