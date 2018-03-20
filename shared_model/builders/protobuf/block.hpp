/**
 * Copyright Soramitsu Co., Ltd. 2018 All Rights Reserved.
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

#ifndef IROHA_PROTO_BLOCK_BUILDER_HPP
#define IROHA_PROTO_BLOCK_BUILDER_HPP

#include "builders/protobuf/builder_templates/block_template.hpp"

namespace shared_model {
  namespace proto {

    using BlockBuilder = TemplateBlockBuilder<>;

    using UnsignedBlockBuilder =
        TemplateBlockBuilder<0,
                             shared_model::validation::DefaultBlockValidator,
                             shared_model::proto::Block>;
  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_PROTO_BLOCK_BUILDER_HPP
