/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_RAW_BLOCK_INSERTION_HPP
#define IROHA_RAW_BLOCK_INSERTION_HPP

#include <memory>
#include <string>

#include <boost/optional.hpp>

#include "logger/logger.hpp"

namespace shared_model {
  namespace interface {
    class Block;
  }
}  // namespace shared_model

namespace iroha {
  namespace main {
    /**
     * Class provide functionality to insert blocks to storage
     * without any validation.
     * This class will be useful for creating test environment
     * and testing pipeline.
     */
    class BlockLoader {
     public:
      explicit BlockLoader(logger::Logger log = logger::log("BlockLoader"));

      /**
       * Parse block from file
       * @param data - raw presenetation of block
       * @return object if operation done successfully, nullopt otherwise
       */
      boost::optional<std::shared_ptr<shared_model::interface::Block>>
      parseBlock(const std::string &data);

      /**
       * Loading file from target path
       * @param path - target file
       * @return string with file content or nullopt
       */
      boost::optional<std::string> loadFile(const std::string &path);

     private:
      logger::Logger log_;
    };

  }  // namespace main
}  // namespace iroha
#endif  // IROHA_RAW_BLOCK_INSERTION_HPP
