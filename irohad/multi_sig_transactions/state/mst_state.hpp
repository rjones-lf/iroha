/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_MST_STATE_HPP
#define IROHA_MST_STATE_HPP

#include <chrono>
#include <queue>
#include <unordered_set>
#include <vector>

#include <boost/optional/optional.hpp>
#include "logger/logger_fwd.hpp"
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
    virtual bool isCompleted(const DataType &batch) const = 0;

    /**
     * Check whether the batch has expired
     * @param batch - object for validation
     * @param current_time - current time
     * @return true, if the batch has expired
     */
    virtual bool isExpired(const DataType &batch,
                           const TimeType &current_time) const = 0;

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
   * Class provides the default behavior for the batch completer.
   * Complete, if all transactions have at least quorum number of signatures.
   * Expired if at least one transaction is expired.
   */
  class DefaultCompleter : public Completer {
   public:
    /**
     * Creates new Completer with a given expiration time for transactions
     * @param expiration_time - expiration time in minutes
     */
    explicit DefaultCompleter(std::chrono::minutes expiration_time);

    bool isCompleted(const DataType &batch) const override;

    bool isExpired(const DataType &tx,
                   const TimeType &current_time) const override;

   private:
    std::chrono::minutes expiration_time_;
  };

  using CompleterType = std::shared_ptr<const Completer>;

  class MstState {
   public:
    // -----------------------------| public api |------------------------------

    /**
     * Create empty state
     * @param completer - strategy for determine completed and expired batches
     * @param transaction_limit - maximum quantity of transactions stored
     * @param log - the logger to use in the new object
     * @return empty mst state
     */
    static MstState empty(const CompleterType &completer,
                          size_t transaction_limit,
                          logger::LoggerPtr log);

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
     * Erase and return expired batches
     * @param current_time - current time
     * @return state with expired batches
     */
    MstState extractExpired(const TimeType &current_time);

    /**
     * Erase expired batches
     * @param current_time - current time
     */
    void eraseExpired(const TimeType &current_time);

    /**
     * Check, if this MST state contains that element
     * @param element to be checked
     * @return true, if state contains the element, false otherwise
     */
    bool contains(const DataType &element) const;

    /// Get the quantity of stored transactions.
    size_t transactionsQuantity() const;

    /// Get the quantity of stored batches.
    size_t batchesQuantity() const;

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

    MstState(const CompleterType &completer,
             size_t transaction_limit,
             logger::LoggerPtr log);

    MstState(const CompleterType &completer,
             size_t transaction_limit,
             const InternalStateType &batches,
             logger::LoggerPtr log);

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

    /**
     * Erase expired batches, optionally returning them.
     * @param current_time - current time
     * @param extracted - optional storage for extracted batches.
     */
    void extractExpiredImpl(const TimeType &current_time,
                            boost::optional<MstState &> extracted);

    // -----------------------------| fields |------------------------------

    CompleterType completer_;

    InternalStateType internal_state_;

    IndexType index_;

    size_t txs_limit_;
    size_t txs_quantity_{0};

    logger::LoggerPtr log_;
  };

}  // namespace iroha

#endif  // IROHA_MST_STATE_HPP
