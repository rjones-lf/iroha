/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
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

      BlockQueryResponseBuilder errorResponse(const std::string &message);

     private:
      iroha::protocol::BlockQueryResponse query_response_;
    };
  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_PROTO_BLOCK_QUERY_RESPONSE_BUILDER_HPP
