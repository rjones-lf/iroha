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

#include <algorithm>
#include <numeric>
#include "validation/impl/stateful_validator_impl.hpp"

namespace iroha {
  namespace validation {

    model::Proposal StatefulValidatorImpl::validate(
        const model::Proposal &proposal,
        ametsuchi::TemporaryWsv &temporaryWsv) {
      auto checking_transaction = [&temporaryWsv](auto &tx, auto &executor,
                                                  auto &query) {
        auto account = temporaryWsv.getAccount(tx.creator_account_id);
        // Check if tx creator has account and has quorum to execute transaction
        if (!account || tx.signatures.size() < account.value().quorum)
          return false;

        // Check if signatures in transaction are account signatory
        auto account_signs = temporaryWsv.getSignatories(tx.creator_account_id);
        if (!account_signs)
          // No signatories found
          return false;

        // TODO: Check if signatures in transaction are valid

        // Validate and execute all commands in transaction
        return std::all_of(
            std::begin(tx.commands), std::end(tx.commands),
            [&query, &account, &executor](auto &command) {
              return command->validate(query, account.value()) &&
                     command->execute(query, executor);
            });

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

      return model::Proposal(
          std::accumulate(txs.begin(), txs.end(), valid, filter));
    }

  }  // namespace validation
}  // namespace iroha