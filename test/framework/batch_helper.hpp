/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_BATCH_HELPER_HPP
#define IROHA_BATCH_HELPER_HPP

#include <boost/range/irange.hpp>
#include "module/shared_model/builders/protobuf/test_transaction_builder.hpp"

namespace framework {
  namespace batch {

    /**
     * Creates transaction builder with set creator
     * @return prepared transaction builder
     */
    auto prepareTransactionBuilder(
        const std::string &creator,
        const size_t &created_time = iroha::time::now()) {
      return TestTransactionBuilder()
          .setAccountQuorum(creator, 1)
          .creatorAccountId(creator)
          .createdTime(created_time)
          .quorum(1);
    }

    /**
     * Create unsigned batch with given fields of transactions: batch type and
     * creator account.
     * @param btype_creator_pairs vector of pairs. First element in every pair
     * is batch type and second is creator account
     * @param now created time for every transaction
     * @return batch with the same size as size of range of pairs
     */
    auto createUnsignedBatchTransactions(
        boost::any_range<
            std::pair<shared_model::interface::types::BatchType, std::string>,
            boost::forward_traversal_tag> btype_creator_pairs,
        size_t now = iroha::time::now()) {
      std::vector<shared_model::interface::types::HashType> reduced_hashes;
      for (const auto &btype_creator : btype_creator_pairs) {
        auto tx = prepareTransactionBuilder(btype_creator.second, now).build();
        reduced_hashes.push_back(tx.reducedHash());
      }

      shared_model::interface::types::SharedTxsCollectionType txs;
      std::for_each(btype_creator_pairs.begin(),
                    btype_creator_pairs.end(),
                    [&now, &txs, &reduced_hashes](const auto &btype_creator) {
                      txs.emplace_back(clone(
                          prepareTransactionBuilder(btype_creator.second, now)
                              .batchMeta(btype_creator.first, reduced_hashes)
                              .build()));
                    });
      return txs;
    }

    /**
     * Creates atomic batch from provided creator accounts
     * @param creators vector of creator account ids
     * @return unsigned batch of the same size as the size of creator account
     * ids
     */
    auto createUnsignedBatchTransactions(
        shared_model::interface::types::BatchType batch_type,
        const std::vector<std::string> &creators,
        size_t now = iroha::time::now()) {
      return createUnsignedBatchTransactions(
          creators
              | boost::adaptors::transformed([&batch_type](const auto creator) {
                  return std::make_pair(batch_type, creator);
                }),
          now);
    }

    /**
     * Creates transaction collection for the batch of given type and size
     * @param batch_type type of the creted transactions
     * @param batch_size size of the created collection of transactions
     * @param now created time for every transactions
     * @return unsigned batch
     */
    auto createUnsignedBatchTransactions(
        shared_model::interface::types::BatchType batch_type,
        uint32_t batch_size,
        size_t now = iroha::time::now()) {
      auto range = boost::irange(0, (int)batch_size);
      std::vector<std::string> creators;

      std::transform(range.begin(),
                     range.end(),
                     std::back_inserter(creators),
                     [](const auto &id) {
                       return std::string("account") + std::to_string(id)
                           + "@domain";
                     });

      return createUnsignedBatchTransactions(batch_type, creators, now);
    }

  }  // namespace batch
}  // namespace framework

#endif  // IROHA_BATCH_HELPER_HPP
