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

#ifndef IROHA_PROTO_BLOCK_QUERY_RESPONSE_BUILDER_HPP
#define IROHA_PROTO_BLOCK_QUERY_RESPONSE_BUILDER_HPP

#include "backend/protobuf/query_responses/proto_block_query_response.hpp"

namespace shared_model {
  namespace proto {
    class BlockQueryResponseBuilder {
     public:
      shared_model::proto::BlockQueryResponse build() &&;

      shared_model::proto::BlockQueryResponse build() &;

      BlockQueryResponseBuilder blockResponse(
          shared_model::interface::Block &block);

      BlockQueryResponseBuilder errorResponse(std::string &message);

     private:
      iroha::protocol::BlockQueryResponse query_response_;
    };
  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_PROTO_BLOCK_QUERY_RESPONSE_BUILDER_HPP
