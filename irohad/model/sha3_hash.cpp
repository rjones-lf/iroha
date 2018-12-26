/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "converters/pb_block_factory.hpp"
#include "converters/pb_common.hpp"
#include "converters/pb_query_factory.hpp"
#include "converters/pb_transaction_factory.hpp"

namespace iroha {
  // TODO: 24.01.2018 @victordrobny: remove factories IR-850
  const static model::converters::PbTransactionFactory tx_factory;
  const static model::converters::PbBlockFactory block_factory;
  const static model::converters::PbQueryFactory query_factory;

  hash256_t hash(const model::Transaction &tx) {
    auto &&pb_dat = tx_factory.serialize(tx);
    return hash(pb_dat);
  }

  hash256_t hash(const model::Block &block) {
    auto &&pb_dat = block_factory.serialize(block);
    return hash(pb_dat.block_v1());
  }

  hash256_t hash(const model::Query &query) {
    std::shared_ptr<const model::Query> qptr(&query, [](auto) {});
    auto &&pb_dat = query_factory.serialize(qptr);
    return hash(*pb_dat);
  }
}  // namespace iroha
