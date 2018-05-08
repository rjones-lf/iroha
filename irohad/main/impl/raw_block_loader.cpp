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

#include <fstream>

#include "converters/protobuf/json_proto_converter.hpp"
#include "backend/protobuf/block.hpp"

namespace iroha {
  namespace main {

    using shared_model::converters::protobuf::jsonToProto;
    using shared_model::interface::Block;

    BlockLoader::BlockLoader() : log_(logger::log("BlockLoader")) {}

    boost::optional<std::shared_ptr<Block>> BlockLoader::parseBlock(
        const std::string &data) {
      return jsonToProto<iroha::protocol::Block>(data) | [](auto &&block) {
        return boost::optional<std::shared_ptr<Block>>(
            std::make_shared<shared_model::proto::Block>(std::move(block)));
      };
    }

    boost::optional<std::string> BlockLoader::loadFile(
        const std::string &path) {
      std::ifstream file(path);
      if (not file) {
        log_->error("Cannot read '" + path + "'");
        return boost::none;
      }
      std::string str((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());
      return str;
    }

  }  // namespace main
}  // namespace iroha
