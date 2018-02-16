/**
 * Copyright Soramitsu Co., Ltd. 2017 All Rights Reserved.
 * http://soramitsu.co.jp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "validation/impl/stateful_validator_impl.hpp"
#include <numeric>
#include <set>
#include "model/account.hpp"

namespace iroha {
  namespace validation {

    StatefulValidatorImpl::StatefulValidatorImpl() {
      log_ = logger::log("SFV");
    }

    model::Proposal StatefulValidatorImpl::validate(
        const model::Proposal &proposal,
        ametsuchi::TemporaryWsv &temporaryWsv) {
      log_->info("transactions in proposal: {}", proposal.transactions.size());
      auto checking_transaction = [this](auto &tx, auto &queries) {
        return (queries.getAccount(tx.creator_account_id) |
                [&](const auto &account) {
                  // Check if tx creator has account and has quorum to execute
                  // transaction
                  return tx.signatures.size() >= account.quorum
                      ? queries.getSignatories(tx.creator_account_id)
                      : nonstd::nullopt;
                }
                |
                [&](const auto &signatories) {
                  // Check if signatures in transaction are account signatory
                  return this->signaturesSubset(tx.signatures, signatories)
                      ? nonstd::make_optional(signatories)
                      : nonstd::nullopt;
                })
            .has_value();
      };

      // Filter only valid transactions
      auto filter = [&temporaryWsv, checking_transaction](auto &acc,
                                                          const auto &tx) {
        auto answer = temporaryWsv.apply(tx, checking_transaction);
        if (answer) {
          acc.push_back(tx);
        }
        return acc;
      };

      auto &txs = proposal.transactions;
      decltype(txs) valid = {};

      model::Proposal validated_proposal(
          std::accumulate(txs.begin(), txs.end(), valid, filter));
      validated_proposal.height = proposal.height;
      log_->info("transactions in verified proposal: {}",
                 validated_proposal.transactions.size());
      return validated_proposal;
    }

    bool StatefulValidatorImpl::signaturesSubset(
        const model::Transaction::SignaturesType &signatures,
        const std::vector<pubkey_t> &public_keys) {
      // TODO 09/10/17 Lebedev: simplify the subset verification IR-510
      // #goodfirstissue
      std::set<pubkey_t> txPubkeys;
      for (auto sign : signatures) {
        txPubkeys.insert(sign.pubkey);
      }
      std::set<pubkey_t> accPubkeys;
      for (auto pubkey : public_keys) {
        accPubkeys.insert(pubkey);
      }
      return std::includes(accPubkeys.begin(),
                           accPubkeys.end(),
                           txPubkeys.begin(),
                           txPubkeys.end());
    }

  }  // namespace validation
}  // namespace iroha
