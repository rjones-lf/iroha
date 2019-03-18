/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ametsuchi/impl/flat_file_block_storage_factory.hpp"

#include "ametsuchi/impl/flat_file_block_storage.hpp"

using namespace iroha::ametsuchi;

FlatFileBlockStorageFactory::FlatFileBlockStorageFactory(
    const std::string &path,
    std::shared_ptr<shared_model::interface::BlockJsonConverter>
        json_block_converter,
    logger::LoggerManagerTreePtr log_manager)
    : path_(path),
      json_block_converter_(std::move(json_block_converter)),
      log_manager_(std::move(log_manager)) {}

std::unique_ptr<BlockStorage> FlatFileBlockStorageFactory::create() {
  return std::make_unique<FlatFileBlockStorage>(
      std::move(FlatFile::create(
                    path_, log_manager_->getChild("FlatFile")->getLogger())
                    .get()),
      json_block_converter_,
      log_manager_->getLogger());
}
