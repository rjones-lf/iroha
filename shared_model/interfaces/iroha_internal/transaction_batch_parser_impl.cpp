/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "interfaces/iroha_internal/transaction_batch_parser_impl.hpp"

#include <boost/range/adaptor/indirected.hpp>
#include <boost/range/combine.hpp>
#include "interfaces/iroha_internal/batch_meta.hpp"
#include "interfaces/transaction.hpp"

namespace {
  template <typename InRange, typename OutRange>
  auto parseBatchesImpl(InRange in_range, const OutRange &out_range) {
    std::vector<OutRange> result;

    auto range = boost::combine(in_range, out_range);
    auto begin = std::begin(range), end = std::end(range);
    while (begin != end) {
      auto next = std::find_if(std::next(begin), end, [begin](const auto &its) {
        auto get_meta = [](auto &tx) { return boost::get<0>(tx).batchMeta(); };
        bool tx_has_meta = static_cast<bool>(get_meta(its)),
             begin_has_meta = static_cast<bool>(get_meta(*begin));

        return not(tx_has_meta and begin_has_meta)
            or (tx_has_meta and begin_has_meta
                and **get_meta(its) != **get_meta(*begin));
      });

      auto it = [](auto &p) { return boost::get<1>(p.get_iterator_tuple()); };
      result.emplace_back(it(begin), it(next));
      begin = next;
    }

    return result;
  }
}  // namespace

namespace shared_model {
  namespace interface {

    std::vector<types::TransactionsForwardCollectionType>
    TransactionBatchParserImpl::parseBatches(
        types::TransactionsForwardCollectionType txs) {
      return parseBatchesImpl(txs, txs);
    }

    std::vector<types::TransactionsCollectionType>
    TransactionBatchParserImpl::parseBatches(
        types::TransactionsCollectionType txs) {
      return parseBatchesImpl(txs, txs);
    }

    std::vector<types::SharedTxsCollectionType>
    TransactionBatchParserImpl::parseBatches(
        const types::SharedTxsCollectionType &txs) {
      return parseBatchesImpl(txs | boost::adaptors::indirected, txs);
    }

  }  // namespace interface
}  // namespace shared_model
