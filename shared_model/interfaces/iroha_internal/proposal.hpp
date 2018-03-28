/**
 * Copyright Soramitsu Co., Ltd. 2018 All Rights Reserved.
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

#ifndef IROHA_SHARED_MODEL_PROPOSAL_HPP
#define IROHA_SHARED_MODEL_PROPOSAL_HPP

#include <boost/range/numeric.hpp>
#include <vector>
#include "interfaces/base/primitive.hpp"
#include "interfaces/common_objects/types.hpp"
#include "interfaces/transaction.hpp"
#include "utils/polymorphic_wrapper.hpp"

#ifndef DISABLE_BACKWARD
#include "model/proposal.hpp"
#endif

namespace shared_model {
  namespace interface {

    class Proposal : public HASHABLE(Proposal) {
     public:
      template <class T>
      using w = detail::PolymorphicWrapper<T>;
      using TransactionContainer = std::vector<w<interface::Transaction>>;

      /**
       * @return transactions
       */
      virtual const std::vector<w<Transaction>> &transactions() const = 0;

      /**
       * @return the height
       */
      virtual types::HeightType height() const = 0;

      /**
       * @return created time
       */
      virtual types::TimestampType createdTime() const = 0;

#ifndef DISABLE_BACKWARD
      iroha::model::Proposal *makeOldModel() const override {
        auto txs =
            boost::accumulate(transactions(),
                              std::vector<iroha::model::Transaction>{},
                              [](auto &&vec, const auto &tx) {
                                std::unique_ptr<iroha::model::Transaction> ptr(
                                    tx->makeOldModel());
                                vec.emplace_back(*ptr);
                                return std::forward<decltype(vec)>(vec);
                              });

        auto oldModel = new iroha::model::Proposal(txs);
        oldModel->height = height();
        oldModel->created_time = createdTime();
        return oldModel;
      }
#endif

      std::string toString() const override {
        return detail::PrettyStringBuilder()
            .init("Proposal")
            .append("height", std::to_string(height()))
            .append("transactions")
            .appendAll(
                transactions(),
                [](auto &transaction) { return transaction->toString(); })
            .finalize();
      }
    };

  }  // namespace interface
}  // namespace shared_model

#endif  // IROHA_SHARED_MODEL_PROPOSAL_HPP
