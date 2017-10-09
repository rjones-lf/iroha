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

#ifndef IROHA_RAW_BLOCK_INSERTION_HPP
#define IROHA_RAW_BLOCK_INSERTION_HPP

#include <memory>
#include <nonstd/optional.hpp>
#include <string>
#include <vector>
#include "ametsuchi/storage.hpp"
#include "logger/logger.hpp"
#include "model/block.hpp"
#include "model/converters/json_block_factory.hpp"

namespace iroha {
  namespace main {
    /**
     * Class provide functionality to insert blocks to storage
     * without any validation.
     * This class will be useful for creating test environment
     * and testing pipeline.
     */
    class BlockInserter {
     public:
      explicit BlockInserter(std::shared_ptr<ametsuchi::MutableFactory> factory);

      /**
       * Parse block from file
       * @param data - raw presenetation of block
       * @return object if operation done successfully, nullopt otherwise
       */
      nonstd::optional<model::Block> parseBlock(std::string data);

      /**
       * Apply block to current state of ledger
       * @param blocks - list of blocks for insertion
       */
      void applyToLedger(const std::vector<model::Block>& blocks);

      /**
       * Additional method
       * Loading file from target path
       * @param path - target file
       * @return string with content or nullopt
       */
      nonstd::optional<std::string> loadFile(std::string path);

     private:
      std::shared_ptr<ametsuchi::MutableFactory> factory_;
      model::converters::JsonBlockFactory block_factory_;

      logger::Logger log_;
    };

  } // namespace main
} // namespace iroha
#endif //IROHA_RAW_BLOCK_INSERTION_HPP
