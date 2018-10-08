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
  /**
   * Zips in_range and out_range, where in_range elements are objects, parses
   * batches based on batchMeta values of in_range, and returns a collection of
   * corresponding sub-ranges of out_range
   */
  template <typename InRange, typename OutRange>
  auto parseBatchesImpl(InRange in_range, const OutRange &out_range) {
    std::vector<OutRange> result;
    auto meta = [](const auto &tx) { return boost::get<0>(tx).batchMeta(); };
    auto has_meta = [&](const auto &tx) { return static_cast<bool>(meta(tx)); };
    auto it = [](auto &p) { return boost::get<1>(p.get_iterator_tuple()); };

    auto range = boost::combine(in_range, out_range);
    auto begin = std::begin(range), end = std::end(range);
    while (begin != end) {
      auto next = std::find_if(std::next(begin), end, [&](const auto &tx) {
        bool tx_has_meta = has_meta(tx), begin_has_meta = has_meta(*begin);

        return not(tx_has_meta and begin_has_meta)
            or (**meta(tx) != **meta(*begin));
      });

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
        types::TransactionsForwardCollectionType txs) const noexcept {
      return parseBatchesImpl(txs, txs);
    }

    std::vector<types::TransactionsCollectionType>
    TransactionBatchParserImpl::parseBatches(
        types::TransactionsCollectionType txs) const noexcept {
      return parseBatchesImpl(txs, txs);
    }

    std::vector<types::SharedTxsCollectionType>
    TransactionBatchParserImpl::parseBatches(
        const types::SharedTxsCollectionType &txs) const noexcept {
      return parseBatchesImpl(txs | boost::adaptors::indirected, txs);
    }

  }  // namespace interface
}  // namespace shared_model
