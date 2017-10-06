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

#ifndef IROHA_MST_STATE_HPP
#define IROHA_MST_STATE_HPP

#include <unordered_set>
#include <memory>
#include <vector>
#include "logger/logger.hpp"
#include "model/transaction.hpp"
#include "multi_sig_transactions/mst_types.hpp"
#include "model/operators/hash.hpp"
#include "common/types.hpp"

namespace iroha {

  class MstState {
   public:
  // ------------------------------| public api |-------------------------------


    using DataType = TransactionType;
    using TimeType = iroha::model::Transaction::TimeType;

    class Completer {
     public:

      /**
       * Verify that transaction is completed
       * @param tx - transaction for verification
       * @return true, if complete
       */
      virtual bool operator()(const DataType tx) const = 0;

      virtual ~Completer() = default;
    };

    /**
     * Class provide default behaviour for transaction completer
     */
    class DefaultCompleter : public Completer {
      bool operator()(const MstState::DataType transaction) const override {
        return transaction->signatures.size() >= transaction->quorum;
      }
    };

    using CompliterType = std::shared_ptr<const Completer>;

    /**
     * Create empty state
     * @param completer - stategy for determine complete transactions
     * and expired signatures
     * @return empty mst state
     */
    static MstState empty(
        const CompliterType &completer = std::make_shared<DefaultCompleter>());

    /**
     * Add transaction to current state
     * @param rhs - transaction for insertion
     * @return State with completed transactions
     */
    MstState operator+=(const DataType &rhs);

    /**
     * Concat internal data of states
     * @param rhs - object for joining
     * @return State is union of this and right states
     */
    MstState operator+=(const MstState &rhs);

    /**
     * Operator provide difference between this and rhs operator
     * @param rhs, state for removing
     * @return State that provide difference between left and right states
     * axiom operators:
     * A + B == B + A
     * A - B == A + (B - A)
     */
    MstState operator-(const MstState &rhs) const;

    /**
     * Provide transactions, that contains in state
     */
    std::vector<DataType> getTransactions() const;

    /**
     * Erase expired transactions
     * @param time - current time
     * @return // todo think about return type
     */
    MstState eraseByTime(const TimeType &time);

   private:
  // ------------------------------| private api |------------------------------

    using InternalStateType = std::unordered_set<DataType,
                                                 iroha::model::PointerTxHasher<DataType>,
                                                 iroha::DereferenceEquals<DataType>>;

    MstState(CompliterType completer);

    MstState(CompliterType completer, InternalStateType transactions);

    void insertOne(MstState &out_state, const DataType &rhs_tx);

  // --------------------------------| fields |---------------------------------

    CompliterType completer_;
    InternalStateType internal_state_;

    logger::Logger log_;
  };

} // namespace iroha
#endif //IROHA_MST_STATE_HPP
