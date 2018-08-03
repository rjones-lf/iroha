/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_PEER_QUERY_FACTORY_HPP
#define IROHA_PEER_QUERY_FACTORY_HPP

#include <boost/optional.hpp>

namespace iroha {
  namespace ametsuchi {

    class PeerQuery;

    class PeerQueryFactory {
     public:
      /**
       * Creates a peer query from the current state.
       * @return Created peer query
       */
      virtual boost::optional<std::shared_ptr<PeerQuery>> createPeerQuery() = 0;

      virtual ~PeerQueryFactory() = default;
    };

  }  // namespace ametsuchi
}  // namespace iroha
#endif  // IROHA_PEER_QUERY_FACTORY_HPP
