/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_TX_CACHE_OUTCOME_HPP
#define IROHA_TX_CACHE_OUTCOME_HPP

#include <functional>

#include <boost/variant.hpp>
#include <cryptography/hash.hpp>

#include "cryptography/hash.hpp"

namespace iroha {
  namespace ametsuchi {

    /**
     * Class represents the outcome of tx presence in a cache
     */
    class TxCacheOutcome {
     public:
      /// shortcut for tx hash type
      using HashType = shared_model::crypto::Hash;

     private:
      /**
       * Hash holder class
       * The class is required only for avoiding duplication in outcome classes
       */
      class HashContainer {
       public:
        /// hash of the transaction
        virtual const HashType &hash() const;

        virtual ~HashContainer() = default;

       private:
        TxCacheOutcome::HashType hash_;
      };

     public:
      /**
       * The class means that corresponding transaction was successfully
       * committed to the ledger
       */
      class OnCommit : public HashContainer {};

      /**
       * The class means that corresponding transaction was rejected by the
       * network
       */
      class OnReject : public HashContainer {};

      /**
       * The class means that corresponding transaction doesn't appear in the
       * ledger
       */
      class OnMiss : public HashContainer {};

      /// Sum type of all possible outcomes for the tx cache
      using OutcomeType = boost::variant<OnCommit, OnReject, OnMiss>;

      virtual const OutcomeType &outcome() const;

      virtual ~TxCacheOutcome() = default;

     protected:
      OutcomeType outcome_;
    };
  }  // namespace ametsuchi
}  // namespace iroha

#endif  // IROHA_TX_CACHE_OUTCOME_HPP
