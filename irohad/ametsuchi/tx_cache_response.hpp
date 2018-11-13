/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_TX_CACHE_RESPONSE_HPP
#define IROHA_TX_CACHE_RESPONSE_HPP

#include <boost/variant.hpp>

#include "cryptography/hash.hpp"

namespace iroha {
  namespace ametsuchi {

    namespace tx_cache_response_details {
      /// shortcut for tx hash type
      using HashType = shared_model::crypto::Hash;

      /**
       * Hash holder class
       * The class is required only for avoiding duplication in concrete
       * response classes
       */
      struct HashContainer {
        HashType hash;
      };
    }  // namespace tx_cache_response_details

    /// The namespace contains concrete result types of transaction cache
    namespace tx_cache_status_responses {
      /**
       * The class means that corresponding transaction was successfully
       * committed in the ledger
       */
      class Committed : public tx_cache_response_details::HashContainer {};

      /**
       * The class means that corresponding transaction was rejected by the
       * network
       */
      class Rejected : public tx_cache_response_details::HashContainer {};

      /**
       * The class means that corresponding transaction doesn't appear in the
       * ledger
       */
      class Missing : public tx_cache_response_details::HashContainer {};
    }  // namespace tx_cache_status_responses

    /// Sum type of all possible concrete responses from the tx cache
    using TxCacheStatusType =
        boost::variant<tx_cache_status_responses::Committed,
                       tx_cache_status_responses::Rejected,
                       tx_cache_status_responses::Missing>;

  }  // namespace ametsuchi
}  // namespace iroha

#endif  // IROHA_TX_CACHE_RESPONSE_HPP
