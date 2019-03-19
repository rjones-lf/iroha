/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ametsuchi/impl/flat_file_block_storage_factory.hpp"

#include "ametsuchi/impl/flat_file_block_storage.hpp"

using namespace iroha::ametsuchi;

FlatFileBlockStorageFactory::FlatFileBlockStorageFactory(
    std::function<std::string()> path_provider,
    std::shared_ptr<shared_model::interface::BlockJsonConverter>
        json_block_converter,
    logger::LoggerManagerTreePtr log_manager)
    : path_(path_provider()),
      json_block_converter_(std::move(json_block_converter)),
      log_manager_(std::move(log_manager)) {}

std::unique_ptr<BlockStorage> FlatFileBlockStorageFactory::create() {
  auto flat_file =
      FlatFile::create(path_, log_manager_->getChild("FlatFile")->getLogger());
  if (not flat_file) {
    return nullptr;
  }
  return std::make_unique<FlatFileBlockStorage>(std::move(flat_file.get()),
                                                json_block_converter_,
                                                log_manager_->getLogger());
}
