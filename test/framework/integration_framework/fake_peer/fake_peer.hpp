/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef INTEGRATION_FRAMEWORK_FAKE_PEER_HPP_
#define INTEGRATION_FRAMEWORK_FAKE_PEER_HPP_

#include <memory>
#include <string>

#include <boost/core/noncopyable.hpp>
#include "framework/integration_framework/fake_peer/yac_network_notifier.hpp"
#include "interfaces/iroha_internal/abstract_transport_factory.hpp"
#include "logger/logger.hpp"
#include "network/impl/async_grpc_client.hpp"

namespace shared_model {
  namespace crypto {
    class Keypair;
  }
  namespace interface {
    class CommonObjectsFactory;
    class Transaction;
    class TransactionBatchParser;
    class TransactionBatchFactory;
  }  // namespace interface
}  // namespace shared_model

namespace iroha {
  namespace protocol {
    class Transaction;
  }
  namespace network {
    class MstTransportGrpc;
    class MstTransportNotification;
  }  // namespace network
  namespace consensus {
    namespace yac {
      class NetworkImpl;
      class YacCryptoProvider;
    }
  }
}  // namespace iroha
class ServerRunner;

namespace integration_framework {

  class FakePeer final : public boost::noncopyable {
   public:
    using TransportFactoryType =
        shared_model::interface::AbstractTransportFactory<
            shared_model::interface::Transaction,
            iroha::protocol::Transaction>;

    FakePeer(
        const std::string &listen_ip,
        size_t internal_port,
        const boost::optional<shared_model::crypto::Keypair> &key,
        const std::shared_ptr<shared_model::interface::Peer> &real_peer,
        const std::shared_ptr<shared_model::interface::CommonObjectsFactory>
            &common_objects_factory,
        std::shared_ptr<TransportFactoryType> transaction_factory,
        std::shared_ptr<shared_model::interface::TransactionBatchParser>
            batch_parser,
        std::shared_ptr<shared_model::interface::TransactionBatchFactory>
            transaction_batch_factory,
        bool agree_all_proposals = true);

    void run();

    void subscribeForMstNotifications(
        std::shared_ptr<iroha::network::MstTransportNotification> notification);

    std::string getAddress() const;

    const shared_model::crypto::Keypair &getKeypair() const;

    void enableAgreeAllProposals();

    void disableAgreeAllProposals();

    rxcpp::observable<YacNetworkNotifier::StateMessagePtr>
    get_yac_states_observable();

    void voteForTheSame(
        const integration_framework::YacNetworkNotifier::StateMessagePtr
            &incoming_votes);

   private:
    using MstTransport = iroha::network::MstTransportGrpc;
    using YacTransport = iroha::consensus::yac::NetworkImpl;
    using AsyncCall = iroha::network::AsyncGrpcClient<google::protobuf::Empty>;

    std::shared_ptr<shared_model::interface::CommonObjectsFactory>
        common_objects_factory_;

    const std::string listen_ip_;
    size_t internal_port_;
    std::unique_ptr<shared_model::crypto::Keypair> keypair_;

    std::shared_ptr<shared_model::interface::Peer>
        this_peer_;  //< this fake instance
    std::shared_ptr<shared_model::interface::Peer>
        real_peer_;  //< the real instance

    std::shared_ptr<AsyncCall> async_call_;

    std::shared_ptr<MstTransport> mst_transport_;
    std::shared_ptr<YacTransport> yac_transport_;
    std::unique_ptr<ServerRunner> internal_server_;

    std::shared_ptr<YacNetworkNotifier> yac_network_notifier_;

    std::shared_ptr<iroha::consensus::yac::YacCryptoProvider> yac_crypto_;

    rxcpp::subscription proposal_agreer_subscription_;

    logger::Logger log_;
  };

}  // namespace integration_framework

#endif /* INTEGRATION_FRAMEWORK_FAKE_PEER_HPP_ */
