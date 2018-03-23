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

#ifndef IROHA_SHARED_MODEL_TRANSACTIONS_RESPONSE_HPP
#define IROHA_SHARED_MODEL_TRANSACTIONS_RESPONSE_HPP

#include "interfaces/base/primitive.hpp"
#include "interfaces/common_objects/types.hpp"
#include "interfaces/transaction.hpp"
#include "utils/polymorphic_wrapper.hpp"
#include "utils/string_builder.hpp"
#include "utils/visitor_apply_for_all.hpp"

#ifndef DISABLE_BACKWARD
#include "model/queries/responses/transactions_response.hpp"
#endif

namespace shared_model {
  namespace interface {
    /**
     * Container of asset, for fetching data.
     */
    class TransactionsResponse : public PRIMITIVE(TransactionsResponse) {
     public:
      /**
       * @return Attached transactions
       */
      virtual types::TransactionsCollectionType transactions() const = 0;

      /**
       * Stringify the data.
       * @return string representation of data.
       */
      std::string toString() const override {
        return detail::PrettyStringBuilder()
            .init("TransactionsResponse")
            .appendAll(transactions(), [](auto &tx) { return tx->toString(); })
            .finalize();
      }

      /**
       * @return true if the data are same.
       */
      bool operator==(const ModelType &rhs) const override {
        return transactions() == rhs.transactions();
      }

#ifndef DISABLE_BACKWARD
      /**
       * Makes old model.
       * @return An allocated old model of transactions response.
       */
      OldModelType *makeOldModel() const override {
        OldModelType *oldModel = new OldModelType();
        oldModel->transactions = rxcpp::observable<>::iterate([this] {
          using OldTxType = iroha::model::Transaction;
          std::vector<OldTxType> ret;
          const auto txs = transactions();
          std::for_each(txs.begin(), txs.end(), [&ret](const auto &tx) {
            auto p = std::shared_ptr<OldTxType>(tx->makeOldModel());
            ret.push_back(*p);
          });
          return ret;
        }());
        return oldModel;
      }

#endif
    };
  }  // namespace interface
}  // namespace shared_model
#endif  // IROHA_SHARED_MODEL_TRANSACTIONS_RESPONSE_HPP
