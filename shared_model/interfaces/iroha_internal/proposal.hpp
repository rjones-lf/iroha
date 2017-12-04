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

#include "interfaces/base/hashable.hpp"
#include "interfaces/transaction.hpp"
#include "model/proposal.hpp"
#include "utils/string_builder.hpp"

#ifndef IROHA_SHARED_MODEL_PROPOSAL_HPP
#define IROHA_SHARED_MODEL_PROPOSAL_HPP
namespace shared_model {
  namespace interface {

    class Proposal : public Hashable<Proposal, iroha::model::Proposal> {
      /// Type of a single Transaction
      using TransactionType = detail::PolymorphicWrapper<Transaction>;

      /// Type of proposal transactions' collection
      using TransactionsCollectionType = std::vector<TransactionType>;

      /**
       * @return collection of proposal's transactions
       */
      virtual const TransactionsCollectionType &transactions() const = 0;

      /**
       * @return number of proposal
       */
      virtual types::HeightType &height() const = 0;

      iroha::model::Proposal *makeOldModel() const override {
        std::vector<iroha::model::Transaction> txs;
        std::for_each(
            transactions().begin(), transactions().end(), [&txs](auto &tx) {
              txs.emplace_back(*tx->makeOldModel());
            });
        iroha::model::Proposal *oldStyleProposal =
            new iroha::model::Proposal(txs);
        oldStyleProposal->height = height();
        return oldStyleProposal;
      }

      std::string toString() const override {
        return detail::PrettyStringBuilder()
            .init("Proposal")
            .append("height", std::to_string(height()))
            .append("transactions")
            .appendAll(transactions(), [](auto &tx) { return tx->toString(); })
            .finalize();
      }
    };

  }  // namespace interface
}  // namespace shared_model
#endif  // IROHA_SHARED_MODEL_PROPOSAL_HPP
