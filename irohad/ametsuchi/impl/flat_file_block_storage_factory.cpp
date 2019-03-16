/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ametsuchi/impl/flat_file_block_storage_factory.hpp"

#include "ametsuchi/impl/flat_file_block_storage.hpp"

using namespace iroha::ametsuchi;

FlatFileBlockStorageFactory::FlatFileBlockStorageFactory(
    logger::LoggerPtr log,
    std::unique_ptr<FlatFile> flat_file,
    std::unique_ptr<shared_model::interface::BlockJsonConverter>
        json_block_converter)
    : log_(std::move(log)),
      flat_file_(std::move(flat_file)),
      json_block_converter_(std::move(json_block_converter)) {}

std::unique_ptr<BlockStorage> FlatFileBlockStorageFactory::create() {
  return std::make_unique<FlatFileBlockStorage>(
      std::move(log_), std::move(flat_file_), std::move(json_block_converter_));
}
