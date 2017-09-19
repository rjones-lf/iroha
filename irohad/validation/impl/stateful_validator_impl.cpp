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

namespace iroha {
  namespace validation {

    StatefulValidatorImpl::StatefulValidatorImpl() {
      log_ = logger::log("SFV");
    }

    model::Proposal StatefulValidatorImpl::validate(
        const model::Proposal &proposal,
        ametsuchi::TemporaryWsv &temporaryWsv) {
      log_->info("transactions in proposal: {}", proposal.transactions.size());
      auto checking_transaction = [](auto &tx, auto &queries) {
        auto account = queries.getAccount(tx.creator_account_id);
        // Check if tx creator has account and has quorum to execute transaction
        if (!account || tx.signatures.size() < account.value().quorum)
          return false;

        // Check if signatures in transaction are account signatory
        auto account_signs = queries.getSignatories(tx.creator_account_id);
        if (not account_signs)
          // No signatories found
          return false;

        // TODO: Check if signatures in transaction are valid
        return true;
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

  }  // namespace validation
}  // namespace iroha
