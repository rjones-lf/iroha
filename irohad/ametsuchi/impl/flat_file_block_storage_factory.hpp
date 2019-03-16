/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_FLAT_FILE_BLOCK_STORAGE_FACTORY_HPP
#define IROHA_FLAT_FILE_BLOCK_STORAGE_FACTORY_HPP

#include "ametsuchi/block_storage_factory.hpp"

#include "ametsuchi/impl/flat_file/flat_file.hpp"
#include "interfaces/iroha_internal/block_json_converter.hpp"
#include "logger/logger_fwd.hpp"

namespace iroha {
  namespace ametsuchi {
    class FlatFileBlockStorageFactory : public BlockStorageFactory {
     public:
      FlatFileBlockStorageFactory(
          logger::LoggerPtr log,
          std::unique_ptr<FlatFile> flat_file,
          std::unique_ptr<shared_model::interface::BlockJsonConverter>
              json_block_converter);
      std::unique_ptr<BlockStorage> create() override;

     private:
      logger::LoggerPtr log_;
      std::unique_ptr<FlatFile> flat_file_;
      std::unique_ptr<shared_model::interface::BlockJsonConverter>
          json_block_converter_;
    };
  }  // namespace ametsuchi
}  // namespace iroha

#endif  // IROHA_FLAT_FILE_BLOCK_STORAGE_FACTORY_HPP
