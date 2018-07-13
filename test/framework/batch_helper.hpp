/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_BATCH_HELPER_HPP
#define IROHA_BATCH_HELPER_HPP

#include "module/shared_model/builders/protobuf/test_transaction_builder.hpp"

namespace framework {
  namespace batch {

    /**
     * Creates transaction builder with set creator
     * @return prepared transaction builder
     */
    auto prepareTransactionBuilder(const std::string &creator) {
      return TestTransactionBuilder().creatorAccountId(creator);
    }

    /**
     * Creates atomic batch from provided creator accounts
     * @param creators vector of creator account ids
     * @return atomic batch of the same size as the size of creator account ids
     */
    auto createUnsignedBatch(shared_model::interface::types::BatchType,
                             std::vector<std::string> creators) {
      std::vector<shared_model::interface::types::HashType> reduced_hashes;

      for (const auto &creator : creators) {
        auto tx = prepareTransactionBuilder(creator).build();
        reduced_hashes.push_back(tx.reducedHash());
      }

      shared_model::interface::types::SharedTxsCollectionType txs;
      std::for_each(
          creators.begin(),
          creators.end(),
          [&txs, &reduced_hashes](const auto &creator) {
            txs.emplace_back(
                clone(prepareTransactionBuilder(creator)
                          .batchMeta(
                              shared_model::interface::types::BatchType::ATOMIC,
                              reduced_hashes)
                          .build()));
          });

      return txs;
    }

  }  // namespace batch
}  // namespace framework

#endif  // IROHA_BATCH_HELPER_HPP
