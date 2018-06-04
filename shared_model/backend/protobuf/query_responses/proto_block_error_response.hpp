/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_SHARED_MODEL_PROTO_BLOCK_ERROR_RESPONSE_HPP
#define IROHA_SHARED_MODEL_PROTO_BLOCK_ERROR_RESPONSE_HPP

#include "backend/protobuf/common_objects/trivial_proto.hpp"
#include "interfaces/query_responses/block_error_response.hpp"
#include "responses.pb.h"
#include "utils/lazy_initializer.hpp"
#include "utils/reference_holder.hpp"

namespace shared_model {
  namespace proto {
    class BlockErrorResponse final
        : public CopyableProto<interface::BlockErrorResponse,
                               iroha::protocol::BlockQueryResponse,
                               BlockErrorResponse> {
     public:
      template <typename QueryResponseType>
      explicit BlockErrorResponse(QueryResponseType &&queryResponse)
          : CopyableProto(std::forward<QueryResponseType>(queryResponse)) {}

      BlockErrorResponse(const BlockErrorResponse &o)
          : BlockErrorResponse(o.proto_) {}

      BlockErrorResponse(BlockErrorResponse &&o)
          : BlockErrorResponse(std::move(o.proto_)) {}

      const std::string &message() const override {
        return *message_;
      }

     private:
      template <typename T>
      using Lazy = detail::LazyInitializer<T>;

      const iroha::protocol::BlockErrorResponse &blockErrorResponse_{
          proto_->error_response()};

      const Lazy<std::string> message_{
          [this] { return blockErrorResponse_.message(); }};
    };
  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_SHARED_MODEL_PROTO_BLOCK_ERROR_RESPONSE_HPP
