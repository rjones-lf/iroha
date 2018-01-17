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

#include "main/raw_block_loader.hpp"
#include "model/converters/json_common.hpp"
#include <fstream>
#include <utility>
#include "common/types.hpp"

namespace iroha {
  namespace main {

    nonstd::optional<model::Block> BlockLoader::parseBlock(std::string data) {
      auto document = model::converters::stringToJson(data);
      if (not document.has_value()) {
        log_->error("Blob parsing failed");
        return nonstd::nullopt;
      }
      return block_factory_.deserialize(document.value());
    }

    nonstd::optional<std::string> BlockLoader::loadFile(std::string path) {
      std::ifstream file(path);
      std::string str((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());
      return str;
    }

  } // namespace main
} // namespace iroha
