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

#ifndef IROHA_PROTO_BLOCKS_QUERY_BUILDER_TEMPLATE_HPP
#define IROHA_PROTO_BLOCKS_QUERY_BUILDER_TEMPLATE_HPP

#include "backend/protobuf/queries/proto_blocks_query.hpp"
#include "builders/protobuf/unsigned_proto.hpp"
#include "interfaces/queries/blocks_query.hpp"
#include "interfaces/common_objects/types.hpp"
#include "interfaces/transaction.hpp"
#include "queries.pb.h"
#include "validators/default_validator.hpp"

namespace shared_model {
  namespace proto {

    /**
     * Template blocks query builder for creating new types of builders by
     * means of replacing template parameters
     * @tparam S -- field counter for checking that all required fields are
     * set
     * @tparam SV -- stateless validator called when build method is invoked
     * @tparam BT -- build type of built object returned by build method
     */
    template <int S = 0,
        typename SV = validation::DefaultBlocksQueryValidator,
        typename BT = UnsignedWrapper<BlocksQuery>>
    class TemplateBlocksQueryBuilder {
     private:
      template <int, typename, typename>
      friend class TemplateBlocksQueryBuilder;

      enum RequiredFields {
        CreatedTime,
        CreatorAccountId,
        QueryCounter,
        TOTAL
      };

      template <int s>
      using NextBuilder = TemplateBlocksQueryBuilder<S | (1 << s), SV, BT>;

      using ProtoBlocksQuery = iroha::protocol::BlocksQuery;

      template <int Sp>
      TemplateBlocksQueryBuilder(const TemplateBlocksQueryBuilder<Sp, SV, BT> &o)
          : query_(o.query_), stateless_validator_(o.stateless_validator_) {}

      /**
       * Make transformation on copied content
       * @tparam Transformation - callable type for changing the copy
       * @param t - transform function for proto object
       * @return new builder with updated state
       */
      template <int Fields, typename Transformation>
      auto transform(Transformation t) const {
        NextBuilder<Fields> copy = *this;
        t(copy.query_);
        return copy;
      }

     public:
      TemplateBlocksQueryBuilder(const SV &validator = SV())
          : stateless_validator_(validator) {}

      auto createdTime(interface::types::TimestampType created_time) const {
        return transform<CreatedTime>([&](auto &qry) {
          auto *meta = qry.mutable_meta();
          meta->set_created_time(created_time);
        });
      }

      auto creatorAccountId(
          const interface::types::AccountIdType &creator_account_id) const {
        return transform<CreatorAccountId>([&](auto &qry) {
          auto *meta = qry.mutable_meta();
          meta->set_creator_account_id(creator_account_id);
        });
      }

      auto queryCounter(interface::types::CounterType query_counter) const {
        return transform<QueryCounter>([&](auto &qry) {
          auto *meta = qry.mutable_meta();
          meta->set_query_counter(query_counter);
        });
      }

      auto build() const {
        static_assert(S == (1 << TOTAL) - 1, "Required fields are not set");
        auto result = BlocksQuery(iroha::protocol::BlocksQuery(query_));
        auto answer = stateless_validator_.validate(result);
        if (answer.hasErrors()) {
          throw std::invalid_argument(answer.reason());
        }
        return BT(std::move(result));
      }

      static const int total = RequiredFields::TOTAL;

     private:
      ProtoBlocksQuery query_;
      SV stateless_validator_;
    };
  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_PROTO_BLOCKS_QUERY_BUILDER_TEMPLATE_HPP
