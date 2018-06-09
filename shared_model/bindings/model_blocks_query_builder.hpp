/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_MODEL_BLOCKS_QUERY_BUILDER_HPP
#define IROHA_MODEL_BLOCKS_QUERY_BUILDER_HPP

#include "builders/protobuf/queries.hpp"
#include "builders/protobuf/unsigned_proto.hpp"

namespace shared_model {
  namespace bindings {

    class ModelBlocksQueryBuilder {
     private:
      template <int Sp>
      explicit ModelBlocksQueryBuilder(
          const proto::TemplateBlocksQueryBuilder<Sp> &o)
          : builder_(o) {}

      proto::TemplateBlocksQueryBuilder<
          (1 << shared_model::proto::TemplateBlocksQueryBuilder<>::total) - 1>
          builder_;

     public:
      ModelBlocksQueryBuilder();

      ModelBlocksQueryBuilder createdTime(
          interface::types::TimestampType created_time);

      ModelBlocksQueryBuilder creatorAccountId(
          const interface::types::AccountIdType &creator_account_id);

      ModelBlocksQueryBuilder queryCounter(
          interface::types::CounterType query_counter);

      proto::UnsignedWrapper<proto::BlocksQuery> build();
    };

  }  // namespace bindings
}  // namespace shared_model

#endif  // IROHA_MODEL_BLOCKS_QUERY_BUILDER_HPP
