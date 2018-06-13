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

    /**
     * Wrapper class for query builder. Designed only for SWIG bindings,
     * don't use in other cases.
     */
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

      /**
       * Sets created time to the blocks query
       * @param created_time time of creation of query
       * @return builder with created time
       */
      ModelBlocksQueryBuilder createdTime(
          interface::types::TimestampType created_time);

      /**
       * Sets creator account id
       * @param creator_account_id
       * @return builder with creator account id
       */
      ModelBlocksQueryBuilder creatorAccountId(
          const interface::types::AccountIdType &creator_account_id);

      /**
       * Sets query counter
       * @param query_counter counter for blocks query
       * @return BlocksQuery with query counter
       */
      ModelBlocksQueryBuilder queryCounter(
          interface::types::CounterType query_counter);

      /**
       * Builds BlocksQuery
       * @return UnsignedWrapper of proto::BlocksQuery
       */
      proto::UnsignedWrapper<proto::BlocksQuery> build();
    };

  }  // namespace bindings
}  // namespace shared_model

#endif  // IROHA_MODEL_BLOCKS_QUERY_BUILDER_HPP
