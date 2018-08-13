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

#include "network/impl/block_loader_service.hpp"
#include "backend/protobuf/block.hpp"

using namespace iroha;
using namespace iroha::ametsuchi;
using namespace iroha::network;

BlockLoaderService::BlockLoaderService(std::shared_ptr<BlockQuery> storage)
    : storage_(std::move(storage)) {
  log_ = logger::log("BlockLoaderService");
}

grpc::Status BlockLoaderService::retrieveBlocks(
    ::grpc::ServerContext *context,
    const proto::BlocksRequest *request,
    ::grpc::ServerWriter<::iroha::protocol::Block> *writer) {
  auto blocks = storage_->getBlocksFrom(request->height());
  std::for_each(blocks.begin(), blocks.end(), [&writer](const auto &block) {
    writer->Write(std::dynamic_pointer_cast<shared_model::proto::Block>(block)
                      ->getTransport());
  });
  return grpc::Status::OK;
}

grpc::Status BlockLoaderService::retrieveBlock(
    ::grpc::ServerContext *context,
    const proto::BlockRequest *request,
    protocol::Block *response) {
  const auto hash = shared_model::crypto::Hash(request->hash());
  if (hash.size() == 0) {
    log_->error("Bad hash in request");
    return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT,
                        "Bad hash provided");
  }

  boost::optional<protocol::Block> result;
  auto blocks = storage_->getBlocksFrom(1);
  std::for_each(
      blocks.begin(), blocks.end(), [&result, &hash](const auto &block) {
        if (block->hash() == hash) {
          result = std::dynamic_pointer_cast<shared_model::proto::Block>(block)
                       ->getTransport();
        }
      });
  if (not result) {
    log_->info("Cannot find block with requested hash");
    return grpc::Status(grpc::StatusCode::NOT_FOUND, "Block not found");
  }
  response->CopyFrom(result.value());
  return grpc::Status::OK;
}
