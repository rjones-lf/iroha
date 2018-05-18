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

      BlockErrorResponse(const BlockErrorResponse &o) : BlockErrorResponse(o.proto_) {}

      BlockErrorResponse(BlockErrorResponse &&o) : BlockErrorResponse(std::move(o.proto_)) {}

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
