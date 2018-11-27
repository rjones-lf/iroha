/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef INTEGRATION_FRAMEWORK_FAKE_PEER_HPP_
#define INTEGRATION_FRAMEWORK_FAKE_PEER_HPP_

#include <memory>
#include <string>

#include <boost/core/noncopyable.hpp>
#include <rxcpp/rx.hpp>
#include "framework/integration_framework/fake_peer/network/mst_message.hpp"
#include "interfaces/iroha_internal/abstract_transport_factory.hpp"
#include "logger/logger.hpp"
#include "network/impl/async_grpc_client.hpp"

namespace shared_model {
  namespace crypto {
    class Keypair;
  }
  namespace interface {
    class CommonObjectsFactory;
    class Proposal;
    class Transaction;
    class TransactionBatch;
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
      class YacHash;
      struct VoteMessage;
    }  // namespace yac
  }    // namespace consensus
  namespace ordering {
    class OrderingGateTransportGrpc;
    class OrderingServiceTransportGrpc;
  }  // namespace ordering
  class MstState;
}  // namespace iroha
class ServerRunner;

namespace integration_framework {
  namespace fake_peer {
    class MstNetworkNotifier;
    class OsNetworkNotifier;
    class OgNetworkNotifier;
    class YacNetworkNotifier;
    class Behaviour;

    /**
     * A lightweight implementation of iroha peer network interface for
     * inter-peer communications testing.
     */
    class FakePeer final : public boost::noncopyable,
                           public std::enable_shared_from_this<FakePeer> {
     public:
      using TransportFactoryType =
          shared_model::interface::AbstractTransportFactory<
              shared_model::interface::Transaction,
              iroha::protocol::Transaction>;
      using MstMessagePtr = std::shared_ptr<MstMessage>;
      using YacMessagePtr = std::shared_ptr<
          const std::vector<iroha::consensus::yac::VoteMessage>>;
      using OgProposalPtr = std::shared_ptr<shared_model::interface::Proposal>;
      using OsBatchPtr =
          std::shared_ptr<shared_model::interface::TransactionBatch>;

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
       * @param gree_all_proposals - whether this peer should agree all
       * proposals
       */
      FakePeer(
          const std::string &listen_ip,
          size_t internal_port,
          const boost::optional<shared_model::crypto::Keypair> &key,
          std::shared_ptr<shared_model::interface::Peer> real_peer,
          const std::shared_ptr<shared_model::interface::CommonObjectsFactory>
              &common_objects_factory,
          std::shared_ptr<TransportFactoryType> transaction_factory,
          std::shared_ptr<shared_model::interface::TransactionBatchParser>
              batch_parser,
          std::shared_ptr<shared_model::interface::TransactionBatchFactory>
              transaction_batch_factory);

      /// Assign the given behaviour to this fake peer.
      FakePeer &setBehaviour(const std::shared_ptr<Behaviour> &behaviour);

      /// Start the fake peer.
      void run();

      /// Get the address:port string of this peer.
      std::string getAddress() const;

      /// Get the keypair of this peer.
      const shared_model::crypto::Keypair &getKeypair() const;

      /// Get the observable of MST states received by this peer.
      rxcpp::observable<MstMessagePtr> getMstStatesObservable();

      /// Get the observable of YAC states received by this peer.
      rxcpp::observable<YacMessagePtr> getYacStatesObservable();

      /// Get the observable of OS batches received by this peer.
      rxcpp::observable<OsBatchPtr> getOsBatchesObservable();

      /// Get the observable of OG proposals received by this peer.
      rxcpp::observable<OgProposalPtr> getOgProposalsObservable();

      /**
       * Send the real peer votes from this peer analogous to the provided ones.
       *
       * @param incoming_votes - the votes to take as the base.
       */
      void voteForTheSame(const YacMessagePtr &incoming_votes);

      /**
       * Make a signature of the provided hash.
       *
       * @param hash - the hash to sign
       */
      std::shared_ptr<shared_model::interface::Signature> makeSignature(
          const shared_model::crypto::Blob &hash) const;

      /// Make a vote from this peer for the provided YAC hash.
      iroha::consensus::yac::VoteMessage makeVote(
          iroha::consensus::yac::YacHash yac_hash);

      /// Send the main peer the given MST state.
      void sendMstState(const iroha::MstState &state);

      /// Send the main peer the given YAC state.
      void sendYacState(
          const std::vector<iroha::consensus::yac::VoteMessage> &state);

      void sendProposal(
          std::unique_ptr<shared_model::interface::Proposal> proposal);

      void sendBatch(
          const std::shared_ptr<shared_model::interface::TransactionBatch>
              &batch);

     private:
      using MstTransport = iroha::network::MstTransportGrpc;
      using YacTransport = iroha::consensus::yac::NetworkImpl;
      using OsTransport = iroha::ordering::OrderingServiceTransportGrpc;
      using OgTransport = iroha::ordering::OrderingGateTransportGrpc;
      using AsyncCall =
          iroha::network::AsyncGrpcClient<google::protobuf::Empty>;

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
      std::shared_ptr<OsTransport> os_transport_;
      std::shared_ptr<OgTransport> og_transport_;

      std::shared_ptr<MstNetworkNotifier> mst_network_notifier_;
      std::shared_ptr<YacNetworkNotifier> yac_network_notifier_;
      std::shared_ptr<OsNetworkNotifier> os_network_notifier_;
      std::shared_ptr<OgNetworkNotifier> og_network_notifier_;

      std::unique_ptr<ServerRunner> internal_server_;

      std::shared_ptr<iroha::consensus::yac::YacCryptoProvider> yac_crypto_;

      std::shared_ptr<Behaviour> behaviour_;

      logger::Logger log_;
    };

  }  // namespace fake_peer
}  // namespace integration_framework

#endif /* INTEGRATION_FRAMEWORK_FAKE_PEER_HPP_ */
