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

#ifndef IROHA_PEER_ORDERER_IMPL_HPP
#define IROHA_PEER_ORDERER_IMPL_HPP

#include <memory>
#include "consensus/yac/yac_peer_orderer.hpp"

namespace iroha {

  namespace ametsuchi {
    class PeerQuery;
  }

  namespace consensus {
    namespace yac {

      class ClusterOrdering;
      class YacHash;

      class PeerOrdererImpl : public YacPeerOrderer {
       public:
        explicit PeerOrdererImpl(
            std::shared_ptr<ametsuchi::PeerQuery> peer_query);

        boost::optional<ClusterOrdering> getInitialOrdering() override;

        boost::optional<ClusterOrdering> getOrdering(
            const YacHash &hash) override;

       private:
        std::shared_ptr<ametsuchi::PeerQuery> query_;
      };

    }  // namespace yac
  }    // namespace consensus
}  // namespace iroha

#endif  // IROHA_PEER_ORDERER_IMPL_HPP
