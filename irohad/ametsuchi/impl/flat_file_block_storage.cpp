/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ametsuchi/impl/flat_file_block_storage.hpp"

#include "backend/protobuf/block.hpp"
#include "common/byteutils.hpp"
#include "logger/logger.hpp"

using namespace iroha::ametsuchi;

FlatFileBlockStorage::FlatFileBlockStorage(
    std::unique_ptr<FlatFile> flat_file,
    std::shared_ptr<shared_model::interface::BlockJsonConverter> json_converter,
    logger::LoggerPtr log)
    : flat_file_storage_(std::move(flat_file)),
      json_converter_(std::move(json_converter)),
      log_(std::move(log)) {}

bool FlatFileBlockStorage::insert(
    std::shared_ptr<const shared_model::interface::Block> block) {
  auto serialized_block = json_converter_->serialize(*block);
  if (auto error =
          boost::get<expected::Error<std::string>>(&serialized_block)) {
    log_->warn("Error while block serialization: {}", error->error);
    return false;
  }

  auto block_json =
      boost::get<expected::Value<std::string>>(serialized_block).value;
  return flat_file_storage_->add(block->height(), stringToBytes(block_json));
}

bool FlatFileBlockStorage::insert(const shared_model::interface::Block &block) {
  return insert(clone(block));
}

boost::optional<std::shared_ptr<const shared_model::interface::Block>>
FlatFileBlockStorage::fetch(
    shared_model::interface::types::HeightType height) const {
  auto storage_block = flat_file_storage_->get(height);
  if (not storage_block) {
    return boost::none;
  }

  auto deserialized_block =
      json_converter_->deserialize(bytesToString(storage_block.get()));
  if (auto error =
          boost::get<expected::Error<std::string>>(&deserialized_block)) {
    log_->warn("Error while block deserialization: {}", error->error);
    return boost::none;
  }

  auto &block =
      boost::get<
          expected::Value<std::unique_ptr<shared_model::interface::Block>>>(
          deserialized_block)
          .value;
  return boost::make_optional<
      std::shared_ptr<const shared_model::interface::Block>>(std::move(block));
}

size_t FlatFileBlockStorage::size() const {
  return flat_file_storage_->blockNumbers().size();
}

void FlatFileBlockStorage::clear() {
  flat_file_storage_->dropAll();
}

void FlatFileBlockStorage::forEach(
    iroha::ametsuchi::BlockStorage::FunctionType function) const {
  for (auto block_id : flat_file_storage_->blockNumbers()) {
    auto block = fetch(block_id);
    BOOST_ASSERT(block);
    function(block.get());
  }
}
