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

    namespace TxCacheResponseDetails {
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
    }  // namespace TxCacheResponseDetails

    /// namespace contains
    namespace TxCacheStatusResponses {
      /**
       * The class means that corresponding transaction was successfully
       * committed in the ledger
       */
      class Committed : public TxCacheResponseDetails::HashContainer {};

      /**
       * The class means that corresponding transaction was rejected by the
       * network
       */
      class Rejected : public TxCacheResponseDetails::HashContainer {};

      /**
       * The class means that corresponding transaction doesn't appear in the
       * ledger
       */
      class Missing : public TxCacheResponseDetails::HashContainer {};
    }  // namespace TxCacheStatusResponses

    /// Sum type of all possible concrete responses from the tx cache
    using TxCacheStatusType = boost::variant<TxCacheStatusResponses::Committed,
                                             TxCacheStatusResponses::Rejected,
                                             TxCacheStatusResponses::Missing>;

  }  // namespace ametsuchi
}  // namespace iroha

#endif  // IROHA_TX_CACHE_RESPONSE_HPP
