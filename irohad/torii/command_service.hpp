/**
 * Copyright Soramitsu Co., Ltd. 2018 All Rights Reserved.
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

#ifndef TORII_COMMAND_SERVICE_HPP
#define TORII_COMMAND_SERVICE_HPP

#include <iostream>
#include <string>
#include <unordered_map>

#include "ametsuchi/block_query.hpp"
#include "cache/cache.hpp"
#include "cryptography/hash.hpp"
#include "endpoint.grpc.pb.h"
#include "endpoint.pb.h"
#include "logger/logger.hpp"
#include "torii/processor/transaction_processor.hpp"

namespace torii {
  /**
   * Actual implementation of sync CommandService.
   */
  class CommandService : public iroha::protocol::CommandService::Service {
   public:
    /**
     * Creates a new instance of CommandService
     * @param pb_factory - model->protobuf and vice versa converter
     * @param tx_processor - processor of received transactions
     * @param block_query - to query transactions outside the cache
     * @param proposal_delay - time of a one proposal propagation.
     */
    CommandService(
        std::shared_ptr<iroha::torii::TransactionProcessor> tx_processor,
        std::shared_ptr<iroha::ametsuchi::BlockQuery> block_query,
        std::chrono::milliseconds proposal_delay);

    /**
     * Disable copying in any way to prevent potential issues with common
     * storage/tx_processor
     */
    CommandService(const CommandService &) = delete;
    CommandService &operator=(const CommandService &) = delete;

    /**
     * Actual implementation of sync Torii in CommandService
     * @param tx - Transaction we've received
     */
    void Torii(const iroha::protocol::Transaction &tx);

    /**
     * Torii call via grpc
     * @param context - call context (see grpc docs for details)
     * @param request - transaction received
     * @param response - no actual response (grpc stub for empty answer)
     * @return - grpc::Status
     */
    virtual grpc::Status Torii(grpc::ServerContext *context,
                               const iroha::protocol::Transaction *request,
                               google::protobuf::Empty *response) override;

    /**
     * Request to retrieve a status of any particular transaction
     * @param request - TxStatusRequest object which identifies transaction
     * uniquely
     * @param response - ToriiResponse which contains a current state of
     * requested transaction
     */
    void Status(const iroha::protocol::TxStatusRequest &request,
                iroha::protocol::ToriiResponse &response);

    /**
     * Status call via grpc
     * @param context - call context
     * @param request - TxStatusRequest object which identifies transaction
     * uniquely
     * @param response - ToriiResponse which contains a current state of
     * requested transaction
     * @return - grpc::Status
     */
    virtual grpc::Status Status(
        grpc::ServerContext *context,
        const iroha::protocol::TxStatusRequest *request,
        iroha::protocol::ToriiResponse *response) override;

    /**
     * Streaming call which will repeatedly send all statuses of requested
     * transaction from its status at the moment of receiving this request to
     * the some final transaction status (which cannot change anymore)
     * @param request- TxStatusRequest object which identifies transaction
     * uniquely
     * @param response_writer - grpc::ServerWriter which can repeatedly send
     * transaction statuses back to the client
     */
    void StatusStream(
        iroha::protocol::TxStatusRequest const &request,
        grpc::ServerWriter<iroha::protocol::ToriiResponse> &response_writer);

    /**
     * StatusStream call via grpc
     * @param context - call context
     * @param request - TxStatusRequest object which identifies transaction
     * uniquely
     * @param response_writer - grpc::ServerWriter which can repeatedly send
     * transaction statuses back to the client
     * @return - grpc::Status
     */
    virtual grpc::Status StatusStream(
        grpc::ServerContext *context,
        const iroha::protocol::TxStatusRequest *request,
        grpc::ServerWriter<iroha::protocol::ToriiResponse> *response_writer)
        override;

   private:
    void checkCacheAndSend(
        const boost::optional<iroha::protocol::ToriiResponse> &resp,
        grpc::ServerWriter<iroha::protocol::ToriiResponse> &response_writer)
        const;

    bool isFinalStatus(const iroha::protocol::TxStatus &status) const;

   private:
    using CacheType = iroha::cache::Cache<shared_model::crypto::Hash,
                                          iroha::protocol::ToriiResponse,
                                          shared_model::crypto::Hash::Hasher>;

    std::shared_ptr<iroha::torii::TransactionProcessor> tx_processor_;
    std::shared_ptr<iroha::ametsuchi::BlockQuery> block_query_;
    std::chrono::milliseconds proposal_delay_;
    std::chrono::milliseconds start_tx_processing_duration_;
    std::shared_ptr<CacheType> cache_;
    logger::Logger log_;
  };

}  // namespace torii

#endif  // TORII_COMMAND_SERVICE_HPP
