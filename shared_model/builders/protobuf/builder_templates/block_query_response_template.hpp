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

#ifndef IROHA_PROTO_BLOCK_QUERY_RESPONSE_BUILDER_TEMPLATE_HPP
#define IROHA_PROTO_BLOCK_QUERY_RESPONSE_BUILDER_TEMPLATE_HPP

#include "backend/protobuf/query_responses/proto_block_query_response.hpp"
#include "builders/protobuf/helpers.hpp"
#include "common/visitor.hpp"
#include "interfaces/common_objects/types.hpp"
#include "responses.pb.h"

namespace shared_model {
  namespace proto {

    /**
     * Template query response builder for creating new types of query response
     * builders by means of replacing template parameters
     * @tparam S -- field counter for checking that all required fields are
     * set
     */
    template <int S = 0>
    class TemplateBlockQueryResponseBuilder {
     public:
      template <int Sp>
      TemplateBlockQueryResponseBuilder(TemplateBlockQueryResponseBuilder<Sp> &&o)
          : query_response_(std::move(o.query_response_)) {}

     private:
      template <class T>
      static constexpr iroha::protocol::ErrorResponse_Reason reason =
          iroha::protocol::ErrorResponse_Reason_STATELESS_INVALID;

      template <int>
      friend class TemplateBlockQueryResponseBuilder;

      enum RequiredFields { QueryResponseField, TOTAL };

      template <int s>
      using NextBuilder = TemplateBlockQueryResponseBuilder<S | (1 << s)>;

      using ProtoBlockQueryResponse = iroha::protocol::BlockQueryResponse;

      template <class T>
      using w = shared_model::detail::PolymorphicWrapper<T>;

      template <int Sp>
      TemplateBlockQueryResponseBuilder(const TemplateBlockQueryResponseBuilder<Sp> &o)
          : query_response_(o.query_response_) {}

      /**
       * Make transformation on copied content
       * @tparam Transformation - callable type for changing the copy
       * @param t - transform function for proto object
       * @return new builder with updated state
       */
      template <int Fields, typename Transformation>
      auto transform(Transformation t) const {
        NextBuilder<Fields> copy = *this;
        t(copy.query_response_);
        return copy;
      }

      /**
       * Make query field transformation on copied object
       * @tparam Transformation - callable type for changing query
       * @param t - transform function for proto query
       * @return new builder with set query
       */
      template <typename Transformation>
      auto queryResponseField(Transformation t) const {
        NextBuilder<QueryResponseField> copy = *this;
        t(copy.query_response_);
        return copy;
      }

     public:
      TemplateBlockQueryResponseBuilder() = default;

      auto blockResponse(
          const interface::Block &block) const {
        return queryResponseField([&](auto &proto_query_response) {
          iroha::protocol::BlockResponse *query_response =
              proto_query_response.mutable_block_response();

          query_response->mutable_block()->mutable_payload()->set_tx_number(block.txsNumber());
          query_response->mutable_block()->mutable_payload()->set_heigh(block.height());
          query_response->mutable_block()->mutable_payload()->set_prev_hash(block.prevHash());

        });
      }

      auto errorResponse(std::string message) {
        return queryResponseField([&](auto &proto_query_response) {
          iroha::protocol::BlockErrorResponse *query_response =
              proto_query_response.mutable_error_response();

          query_response->set_message(message);
        });
      }

      BlockQueryResponse build() const {
        static_assert(S == (1 << TOTAL) - 1, "Required fields are not set");
        auto result =
            BlockQueryResponse(iroha::protocol::BlockQueryResponse(query_response_));
        return BlockQueryResponse(std::move(result));
      }

      static const int total = RequiredFields::TOTAL;

     private:
      ProtoBlockQueryResponse query_response_;
    };
  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_PROTO_BLOCK_QUERY_RESPONSE_BUILDER_TEMPLATE_HPP
