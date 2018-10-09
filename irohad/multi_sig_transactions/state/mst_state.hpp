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
   * Completer is strategy for verification batches on
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
     * Operator checks whether the batch has expired
     * @param batch - object for validation
     * @param time - current time
     * @return true, if the batch has expired
     */
    virtual bool operator()(const DataType &batch,
                            const TimeType &time) const = 0;

    virtual ~Completer() = default;
  };

  /**
   * Class provides operator() for batch comparison
   */
  class BatchHashEquality {
   public:
    /**
     * The function used to compare batches for equality:
     * check only hashes of batches, without signatures
     */
    bool operator()(const DataType &left_tx, const DataType &right_tx) const;
  };

  /**
   * Class provides the default behavior for the batch completer:
   * complete, if all transactions have at least quorum number of signatures
   */
  class DefaultCompleter : public Completer {
    bool operator()(const DataType &batch) const override;

    bool operator()(const DataType &tx, const TimeType &time) const override;
  };

  using CompleterType = std::shared_ptr<const Completer>;

  class MstState {
   public:
    // -----------------------------| public api |------------------------------

    /**
     * Create empty state
     * @param completer - strategy for determine completed and expired batches
     * @return empty mst state
     */
    static MstState empty(
        const CompleterType &completer = std::make_shared<DefaultCompleter>());

    /**
     * Add batch to current state
     * @param rhs - batch for insertion
     * @return States with completed and updated batches
     */
    StateUpdateResult operator+=(const DataType &rhs);

    /**
     * Concat internal data of states
     * @param rhs - object for merging
     * @return States with completed and updated batches
     */
    StateUpdateResult operator+=(const MstState &rhs);

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
     * @return true, if there is no batches inside
     */
    bool isEmpty() const;

    /**
     * Compares two different MstState's
     * @param rhs - MstState to compare
     * @return true is rhs equal to this or false otherwise
     */
    bool operator==(const MstState &rhs) const;

    /**
     * @return the batches from the state
     */
    std::unordered_set<DataType,
                       iroha::model::PointerBatchHasher,
                       BatchHashEquality>
    getBatches() const;

    /**
     * Erase expired batches
     * @param time - current time
     * @return state with expired batches
     */
    MstState eraseByTime(const TimeType &time);

   private:
    // --------------------------| private api |------------------------------

    /**
     * Class provides operator < for batches
     */
    class Less {
     public:
      bool operator()(const DataType &left, const DataType &right) const;
    };

    using InternalStateType =
        std::unordered_set<DataType,
                           iroha::model::PointerBatchHasher,
                           BatchHashEquality>;

    using IndexType =
        std::priority_queue<DataType, std::vector<DataType>, Less>;

    MstState(const CompleterType &completer);

    MstState(const CompleterType &completer,
             const InternalStateType &transactions);

    /**
     * Insert batch in own state and push it in out_completed_state or
     * out_updated_state
     * @param state_update consists of states with updated and completed batches
     * @param rhs_tx - batch for insert
     */
    void insertOne(StateUpdateResult &state_update, const DataType &rhs_tx);

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
