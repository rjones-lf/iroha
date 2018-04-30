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

#ifndef IROHA_BLOCK_LOADER_SERVICE_HPP
#define IROHA_BLOCK_LOADER_SERVICE_HPP

#include "ametsuchi/block_query.hpp"
#include "loader.grpc.pb.h"
#include "logger/logger.hpp"

namespace iroha {
  namespace network {
    class BlockLoaderService : public proto::Loader::Service {
     public:
      explicit BlockLoaderService(
          std::shared_ptr<ametsuchi::BlockQuery> storage);

      grpc::Status retrieveBlocks(
          ::grpc::ServerContext *context,
          const proto::BlocksRequest *request,
          ::grpc::ServerWriter<protocol::Block> *writer) override;

      grpc::Status retrieveBlock(::grpc::ServerContext *context,
                                 const proto::BlockRequest *request,
                                 protocol::Block *response) override;

     private:
      std::shared_ptr<ametsuchi::BlockQuery> storage_;
      logger::Logger log_;
    };
  }  // namespace network
}  // namespace iroha

#endif  // IROHA_BLOCK_LOADER_SERVICE_HPP
