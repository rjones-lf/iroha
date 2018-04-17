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
#include <string>

#include <boost/optional.hpp>

#include "logger/logger.hpp"

namespace shared_model {
  namespace interface {
    class Block;
  }
}

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
      BlockLoader();

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
