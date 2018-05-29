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

#include "builders/protobuf/query_responses/proto_block_query_response_builder.hpp"

namespace shared_model {
  namespace proto {

    shared_model::proto::BlockQueryResponse BlockQueryResponseBuilder::build()
        && {
      return shared_model::proto::BlockQueryResponse(
          std::move(query_response_));
    }

    shared_model::proto::BlockQueryResponse BlockQueryResponseBuilder::build()
        & {
      return shared_model::proto::BlockQueryResponse(
          iroha::protocol::BlockQueryResponse(query_response_));
    }

    BlockQueryResponseBuilder BlockQueryResponseBuilder::blockResponse(
        shared_model::interface::Block &block) {
      BlockQueryResponseBuilder copy(*this);
      shared_model::proto::Block &proto_block =
          static_cast<shared_model::proto::Block &>(block);
      iroha::protocol::BlockResponse *response =
          new iroha::protocol::BlockResponse();
      response->set_allocated_block(
          new iroha::protocol::Block(proto_block.getTransport()));
      copy.query_response_.set_allocated_block_response(response);
      return copy;
    }

    BlockQueryResponseBuilder BlockQueryResponseBuilder::errorResponse(
        std::string &message) {
      BlockQueryResponseBuilder copy(*this);
      iroha::protocol::BlockErrorResponse *response =
          new iroha::protocol::BlockErrorResponse();
      response->set_message(message);
      copy.query_response_.set_allocated_error_response(response);
      return copy;
    }
  }  // namespace proto
}  // namespace shared_model
