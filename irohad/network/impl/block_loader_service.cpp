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

#include "common/byteutils.hpp"

using namespace iroha;
using namespace iroha::ametsuchi;
using namespace iroha::model;
using namespace iroha::model::converters;
using namespace iroha::network;

BlockLoaderService::BlockLoaderService(std::shared_ptr<BlockQuery> storage)
    : storage_(std::move(storage)) {
  log_ = logger::log("BlockLoaderService");
}

grpc::Status BlockLoaderService::retrieveBlocks(
    ::grpc::ServerContext *context,
    const proto::BlocksRequest *request,
    ::grpc::ServerWriter<::iroha::protocol::Block> *writer) {
  storage_->getBlocksFrom(request->height())
      .map([this](auto block) {
        return factory_.serialize(
            *std::unique_ptr<iroha::model::Block>(block->makeOldModel()));
      })
      .as_blocking()
      .subscribe([writer](auto block) { writer->Write(block); });
  return grpc::Status::OK;
}

grpc::Status BlockLoaderService::retrieveBlock(
    ::grpc::ServerContext *context,
    const proto::BlockRequest *request,
    protocol::Block *response) {
  const auto hash = stringToBlob<Block::HashType::size()>(request->hash());
  if (not hash) {
    log_->error("Bad hash in request, {}",
                bytestringToHexstring(request->hash()));
    return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT,
                        "Bad hash provided");
  }

  boost::optional<protocol::Block> result;
  storage_->getBlocksFrom(1)
      .filter([hash](auto block) {
        return shared_model::crypto::toBinaryString(block->hash())
            == hash->to_string();
      })
      .map([this](auto block) {
        return factory_.serialize(
            *std::unique_ptr<iroha::model::Block>(block->makeOldModel()));
      })
      .as_blocking()
      .subscribe([&result](auto block) { result = block; });
  if (not result) {
    log_->info("Cannot find block with requested hash");
    return grpc::Status(grpc::StatusCode::NOT_FOUND, "Block not found");
  }
  response->CopyFrom(result.value());
  return grpc::Status::OK;
}
