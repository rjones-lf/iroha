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
        bool tx_has_meta = boost::get<0>(its).batchMeta(),
             begin_has_meta = boost::get<0>(*begin).batchMeta();

        return not(tx_has_meta and begin_has_meta)
            or (tx_has_meta and begin_has_meta
                and **boost::get<0>(its).batchMeta()
                    != **boost::get<0>(*begin).batchMeta());
      });

      result.emplace_back(boost::get<1>(begin.get_iterator_tuple()),
                          boost::get<1>(next.get_iterator_tuple()));
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
