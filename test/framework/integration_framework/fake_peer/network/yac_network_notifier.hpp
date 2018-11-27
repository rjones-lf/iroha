/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef FAKE_PEER_YAC_NETWORK_NOTIFIER_HPP_
#define FAKE_PEER_YAC_NETWORK_NOTIFIER_HPP_

#include "consensus/yac/transport/yac_network_interface.hpp"

#include <rxcpp/rx.hpp>

#include "framework/integration_framework/fake_peer/types.hpp"

namespace integration_framework {
  namespace fake_peer {

    class YacNetworkNotifier final
        : public iroha::consensus::yac::YacNetworkNotifications {
     public:
      using StateMessage = std::vector<iroha::consensus::yac::VoteMessage>;

      void onState(StateMessage state) override;

      rxcpp::observable<YacMessagePtr> get_observable();

     private:
      rxcpp::subjects::subject<YacMessagePtr> votes_subject_;
    };

  }  // namespace fake_peer
}  // namespace integration_framework

#endif /* FAKE_PEER_YAC_NETWORK_NOTIFIER_HPP_ */
