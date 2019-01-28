/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef FAKE_PEER_MST_NETWORK_NOTIFIER_HPP_
#define FAKE_PEER_MST_NETWORK_NOTIFIER_HPP_

#include "network/mst_transport.hpp"

#include <rxcpp/rx.hpp>
#include "framework/integration_framework/fake_peer/network/mst_message.hpp"
#include "framework/integration_framework/fake_peer/types.hpp"

namespace integration_framework {
  namespace fake_peer {

    class MstNetworkNotifier final
        : public iroha::network::MstTransportNotification {
     public:
      void onNewState(const shared_model::crypto::PublicKey &from,
                      const iroha::MstState &new_state) override;

      rxcpp::observable<MstMessageSPtr> getObservable();

     private:
      rxcpp::subjects::subject<MstMessageSPtr> mst_subject_;
    };

  }  // namespace fake_peer
}  // namespace integration_framework

#endif /* FAKE_PEER_MST_NETWORK_NOTIFIER_HPP_ */
