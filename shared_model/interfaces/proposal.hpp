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


#ifndef IROHA_SHARED_MODEL_PROPOSAL_HPP
#define IROHA_SHARED_MODEL_PROPOSAL_HPP


#include <vector>
#include "interfaces/base/primitive.hpp"
#include "interfaces/transaction.hpp"
#include "interfaces/common_objects/types.hpp"
#include "utils/polymorphic_wrapper.hpp"

#include "model/proposal.hpp"


namespace shared_model {
  namespace interface {

    class Proposal
            : public Primitive<Proposal, iroha::model::Proposal> {
    public:

      using wTransaction = detail::PolymorphicWrapper<Transaction>;
      /**
       * @return transactions
       */
      virtual const std::vector<wTransaction> &transactions() const = 0;

      /**
       * @return the height
       */
      virtual uint64_t height() const = 0;

      iroha::model::Proposal *makeOldModel() const override {

        std::vector<iroha::model::Transaction> txs;
        for(const auto& tx: transactions()){
          std::unique_ptr<iroha::model::Transaction> ptr(tx->makeOldModel());
          txs.push_back(*ptr);
        }
        auto oldModel = new iroha::model::Proposal(txs);
        return oldModel;
      }

//      std::string toString() const override {
//        return detail::PrettyStringBuilder()
//                .init("Proposal")
//                .append("height", std::to_string(transactions()))
//                .appendAll(transactions(),
//                           [](auto &transaction) { return transaction->toString(); })
//                .finalize();
//      }

//      bool operator==(const ModelType &rhs) const override {
//        return transactions() == rhs.transactions() and height() == rhs.height();
//      }

    };



  }  // namespace interface
}  // namespace shared_model

#endif //IROHA_PROPOSAL_HPP
