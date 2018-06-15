/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "bindings/model_blocks_query_builder.hpp"

namespace shared_model {
  namespace bindings {

    ModelBlocksQueryBuilder::ModelBlocksQueryBuilder() {
      *this = creatorAccountId("").createdTime(0).queryCounter(0);
    }

    ModelBlocksQueryBuilder ModelBlocksQueryBuilder::createdTime(
        interface::types::TimestampType created_time) {
      return ModelBlocksQueryBuilder{builder_.createdTime(created_time)};
    }

    ModelBlocksQueryBuilder ModelBlocksQueryBuilder::creatorAccountId(
        const interface::types::AccountIdType &creator_account_id) {
      return ModelBlocksQueryBuilder(
          builder_.creatorAccountId(creator_account_id));
    }

    ModelBlocksQueryBuilder ModelBlocksQueryBuilder::queryCounter(
        interface::types::CounterType query_counter) {
      return ModelBlocksQueryBuilder(builder_.queryCounter(query_counter));
    }

    proto::UnsignedWrapper<proto::BlocksQuery>
    ModelBlocksQueryBuilder::build() {
      return builder_.build();
    }

  }  // namespace bindings
}  // namespace shared_model
