/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <boost/format.hpp>
#include <boost/range/algorithm/find.hpp>

#include "batch_order_validator.hpp"
#include "interfaces/iroha_internal/batch_meta.hpp"

namespace shared_model {
  namespace validation {
    std::string BatchOrderValidator::canFollow(
        boost::optional<const interface::Transaction &> tr1,
        boost::optional<const interface::Transaction &> tr2) const {
      boost::optional<std::shared_ptr<interface::BatchMeta>> batch1 =
          tr1 ? tr1->batch_meta() : boost::none;
      boost::optional<std::shared_ptr<interface::BatchMeta>> batch2 =
          tr2 ? tr2->batch_meta() : boost::none;
      // both transactions are not a part of any batch
      if (not batch1 and not batch2) {
        return "";
      }
      // beginning of a batch
      if (not batch1) {
        if (batch2.get()->transactionHashes().size() == 0) {
          return (boost::format("Tx %s has a batch of 0 transactions")
                  % tr2->hash().hex())
              .str();
        }
        if (batch2.get()->transactionHashes().front() != tr2->reduced_hash()) {
          return (boost::format("Tx %s is a first transaction of a batch, but "
                                "it's reduced hash %s doesn't match the first "
                                "reduced hash in batch %s")
                  % tr2->hash().hex()
                  % batch2.get()->transactionHashes().front().hex()
                  % tr2->reduced_hash().hex())
              .str();
        }
        return "";
      }
      // end of a batch
      if (not batch2) {
        if (batch1.get()->transactionHashes().back() != tr1->reduced_hash()) {
          return (boost::format("Tx %s is a last transaction of a batch, but "
                                "it's reduced hash %s doesn't match the last "
                                "reduced hash in batch %s")
                  % tr1->hash().hex()
                  % batch1.get()->transactionHashes().back().hex()
                  % tr1->reduced_hash().hex())
              .str();
        }
        return "";
      }
      // inside of a batch
      auto it1 =
          boost::find(batch1.get()->transactionHashes(), tr1->reduced_hash());
      auto it2 =
          boost::find(batch2.get()->transactionHashes(), tr2->reduced_hash());
      if (it1 == end(batch1.get()->transactionHashes())
          or it2 == end(batch2.get()->transactionHashes())
          or next(it1) == end(batch1.get()->transactionHashes())
          or *next(it1) != *it2) {
        // end of the bach and beginning of the next
        if (canFollow(tr1, boost::none) == ""
            and canFollow(boost::none, tr2) == "") {
          return "";
        }
        return (boost::format("Tx %s is followed by %s, but their reduced"
                              "hashed doesn't follow each other in their batch"
                              "meta")
                % tr1->hash().hex() % tr2->hash().hex())
            .str();
      }
      if (**batch1 != **batch2) {
        return (boost::format("Tx %s and %s are part of the same batch, but "
                              "their batch metas doesn't match")
                % tr1->hash().hex() % tr2->hash().hex())
            .str();
      }
      return "";
    }

    Answer BatchOrderValidator::validate(
        const interface::types::TransactionsForwardCollectionType &transactions)
        const {
      Answer res;
      ReasonsGroupType reason;
      reason.first = "Transaction order";
      boost::optional<const interface::Transaction &> prev_transaction =
          boost::none;

      for (auto &transaction : transactions) {
        auto message = canFollow(prev_transaction, transaction);
        if (message != "") {
          reason.second.push_back(message);
        }
        prev_transaction = transaction;
      }
      auto message = canFollow(prev_transaction, boost::none);
      if (message != "") {
        reason.second.push_back(message);
      }
      if (not reason.second.empty()) {
        res.addReason(std::move(reason));
      }
      return res;
    }
  }  // namespace validation
}  // namespace shared_model
