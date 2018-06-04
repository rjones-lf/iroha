/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_SHARED_MODEL_PROTO_BLOCK_RESPONSE_HPP
#define IROHA_SHARED_MODEL_PROTO_BLOCK_RESPONSE_HPP

#include "backend/protobuf/block.hpp"
#include "backend/protobuf/common_objects/trivial_proto.hpp"
#include "interfaces/query_responses/block_response.hpp"
#include "responses.pb.h"
#include "utils/lazy_initializer.hpp"
#include "utils/reference_holder.hpp"

namespace shared_model {
  namespace proto {
    class BlockResponse final
        : public CopyableProto<interface::BlockResponse,
                               iroha::protocol::BlockQueryResponse,
                               BlockResponse> {
     public:
      template <typename QueryResponseType>
      explicit BlockResponse(QueryResponseType &&queryResponse)
          : CopyableProto(std::forward<QueryResponseType>(queryResponse)) {}

      BlockResponse(const BlockResponse &o) : BlockResponse(o.proto_) {}

      BlockResponse(BlockResponse &&o) : BlockResponse(std::move(o.proto_)) {}

      const Block &block() const override {
        return *block_;
      }

     private:
      template <typename T>
      using Lazy = detail::LazyInitializer<T>;

      const iroha::protocol::BlockResponse &blockResponse_{
          proto_->block_response()};

      const Lazy<Block> block_{
          [this] { return Block(blockResponse_.block()); }};
    };
  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_SHARED_MODEL_PROTO_BLOCK_RESPONSE_HPP
