/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
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
          copy.query_response_.mutable_block_response();
      response->set_allocated_block(
          new iroha::protocol::Block(proto_block.getTransport()));
      return copy;
    }

    BlockQueryResponseBuilder BlockQueryResponseBuilder::errorResponse(
        const std::string &message) {
      BlockQueryResponseBuilder copy(*this);
      iroha::protocol::BlockErrorResponse *response =
          copy.query_response_.mutable_block_error_response();
      response->set_message(message);
      return copy;
    }
  }  // namespace proto
}  // namespace shared_model
