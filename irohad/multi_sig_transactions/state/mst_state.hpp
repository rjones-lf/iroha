/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_MST_STATE_HPP
#define IROHA_MST_STATE_HPP

#include <queue>
#include <unordered_set>
#include <vector>
#include "logger/logger.hpp"

#include "multi_sig_transactions/hash.hpp"
#include "multi_sig_transactions/mst_types.hpp"

namespace iroha {

  /**
   * Completer is strategy for verification transactions on
   * completeness and expiration
   */
  class Completer {
   public:
    /**
     * Verify that batch is completed
     * @param batch - target object for verification
     * @return true, if complete
     */
    virtual bool operator()(const DataType &batch) const = 0;

    /**
     * Check that invariant for time expiration for batch
     * @param batch - object for validation
     * @param time - current time
     * @return true, if transaction was expired
     */
    virtual bool operator()(const DataType &batch,
                            const TimeType &time) const = 0;

    virtual ~Completer() = default;
  };

  /**
   * Equality semantic for batches:
   * check only payloads of transactions, without signatures
   */
  class BatchHashEquality {
   public:
    bool operator()(const DataType &left_tx, const DataType &right_tx) const {
      return left_tx->reducedHash() == right_tx->reducedHash();
    }
  };

  /**
   * Class provide default behaviour for transaction completer
   */
  class DefaultCompleter : public Completer {
    bool operator()(const DataType &batch) const override {
      return std::accumulate(
          batch->transactions().begin(),
          batch->transactions().end(),
          true,
          [](bool value, const auto &tx) {
            return value and boost::size(tx->signatures()) >= tx->quorum();
          });
    }

    bool operator()(const DataType &tx, const TimeType &time) const override {
      return false;
    }
  };

  using CompleterType = std::shared_ptr<const Completer>;

  class MstState {
   public:
    // -----------------------------| public api |------------------------------

    /**
     * Create empty state
     * @param completer - stategy for determine complete transactions
     * and expired signatures
     * @return empty mst state
     */
    static MstState empty(
        const CompleterType &completer = std::make_shared<DefaultCompleter>());

    /**
     * Add transaction to current state
     * @param rhs - transaction for insertion
     * @return State with completed transactions
     */
    MstState operator+=(const DataType &rhs);

    /**
     * Concat internal data of states
     * @param rhs - object for merging
     * @return State with completed trasactions
     */
    MstState operator+=(const MstState &rhs);

    /**
     * Operator provide difference between this and rhs operator
     * @param rhs, state for removing
     * @return State that provide difference between left and right states
     * axiom operators:
     * A V B == B V A
     * A V B == B V (A \ B)
     */
    MstState operator-(const MstState &rhs) const;

    /**
     * @return true, if there is no transactions inside
     */
    bool isEmpty() const;

    /**
     * Compares two different MstState's
     * @param rhs - MstState to compare
     * @return true is rhs equal to this or false otherwise
     */
    bool operator==(const MstState &rhs) const;

    /**
     * Provide batches, which contains in state
     */
    std::vector<DataType> getBatches() const;

    /**
     * Erase expired transactions
     * @param time - current time
     * @return state with expired transactions
     */
    MstState eraseByTime(const TimeType &time);

   private:
    // --------------------------| private api |------------------------------

    /**
     * Class for providing operator < for transactions
     */
    class Less {
     public:
      bool operator()(const DataType &left, const DataType &right) const {
        return left->transactions().at(0)->createdTime()
            < right->transactions().at(0)->createdTime();
      }
    };

    using InternalStateType =
        std::unordered_set<DataType,
                           iroha::model::PointerBatchHasher<DataType>,
                           BatchHashEquality>;

    using IndexType =
        std::priority_queue<DataType, std::vector<DataType>, Less>;

    MstState(const CompleterType &completer);

    MstState(const CompleterType &completer,
             const InternalStateType &transactions);

    /**
     * Insert transaction in own state and push it in out_state if required
     * @param out_state - state for inserting completed transactions
     * @param rhs_tx - transaction for insert
     */
    void insertOne(MstState &out_state, const DataType &rhs_tx);

    /**
     * Insert new value in state with keeping invariant
     * @param rhs_tx - data for insertion
     */
    void rawInsert(const DataType &rhs_tx);

    // -----------------------------| fields |------------------------------

    CompleterType completer_;

    InternalStateType internal_state_;

    IndexType index_;

    logger::Logger log_;
  };

}  // namespace iroha
#endif  // IROHA_MST_STATE_HPP
