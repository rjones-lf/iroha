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
  namespace ametsuchi {
    class TxPresenceCache;
  }  // namespace ametsuchi
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
      class YacHash;
    }  // namespace yac
  }    // namespace consensus
}  // namespace iroha
class ServerRunner;

namespace integration_framework {

  /**
   * A lightweight implementation of iroha peer network interface for inter-peer
   * communications testing.
   */
  class FakePeer final : public boost::noncopyable {
   public:
    using TransportFactoryType =
        shared_model::interface::AbstractTransportFactory<
            shared_model::interface::Transaction,
            iroha::protocol::Transaction>;

    /**
     * Constructor.
     *
     * @param listen_ip - IP on which this fake peer should listen
     * @param internal_port - the port for internal commulications
     * @param key - the keypair of this peer
     * @param real_peer - the main tested peer managed by ITF
     * @param common_objects_factory - common_objects_factory
     * @param transaction_factory - transaction_factory
     * @param batch_parser - batch_parser
     * @param transaction_batch_factory - transaction_batch_factory
     * @param gree_all_proposals - whether this peer should agree all proposals
     */
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
        std::shared_ptr<iroha::ametsuchi::TxPresenceCache> tx_presence_cache,
        bool agree_all_proposals = true);

    /// Start the fake peer.
    void run();

    /**
     * Subscribe for mst notifications.
     *
     * @param notification - the object to subscribe
     */
    void subscribeForMstNotifications(
        std::shared_ptr<iroha::network::MstTransportNotification> notification);

    /// Get the address:port string of this peer.
    std::string getAddress() const;

    /// Get the keypair of this peer.
    const shared_model::crypto::Keypair &getKeypair() const;

    /// Make this peer agree all proposals.
    void enableAgreeAllProposals();

    /// Stop this peer from agreeing all proposals.
    void disableAgreeAllProposals();

    /// Get the observable of YAC states received by this peer.
    rxcpp::observable<YacNetworkNotifier::StateMessagePtr>
    get_yac_states_observable();

    /**
     * Send the real peer votes from this peer analogous to the provided ones.
     *
     * @param incoming_votes - the votes to take as the base.
     */
    void voteForTheSame(
        const integration_framework::YacNetworkNotifier::StateMessagePtr
            &incoming_votes);

    /**
     * Make a signature of the provided hash.
     *
     * @param hash - the hash to sign
     */
    std::shared_ptr<shared_model::interface::Signature> makeSignature(
        const shared_model::crypto::Blob &hash) const;

    /// Make a vote from this peer for the provided YAC hash.
    iroha::consensus::yac::VoteMessage makeVote(
        const iroha::consensus::yac::YacHash &yac_hash);

    /// Send the main peer the given YAC state.
    void sendYacState(
        const std::vector<iroha::consensus::yac::VoteMessage> &state);

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
        this_peer_;  ///< this fake instance
    std::shared_ptr<shared_model::interface::Peer>
        real_peer_;  ///< the real instance

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
