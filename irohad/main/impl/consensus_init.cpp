/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "main/impl/consensus_init.hpp"

#include "consensus/yac/consistency_model.hpp"
#include "consensus/yac/impl/peer_orderer_impl.hpp"
#include "consensus/yac/impl/timer_impl.hpp"
#include "consensus/yac/impl/yac_crypto_provider_impl.hpp"
#include "consensus/yac/impl/yac_gate_impl.hpp"
#include "consensus/yac/impl/yac_hash_provider_impl.hpp"
#include "consensus/yac/storage/buffered_cleanup_strategy.hpp"
#include "consensus/yac/storage/yac_proposal_storage.hpp"
#include "consensus/yac/transport/impl/network_impl.hpp"
#include "logger/logger_manager.hpp"
#include "network/impl/grpc_channel_builder.hpp"

using namespace iroha::consensus::yac;

namespace {
  auto createPeerOrderer(
      std::shared_ptr<iroha::ametsuchi::PeerQueryFactory> peer_query_factory) {
    return std::make_shared<PeerOrdererImpl>(peer_query_factory);
  }

  auto createCryptoProvider(
      const shared_model::crypto::Keypair &keypair,
      std::shared_ptr<shared_model::interface::CommonObjectsFactory>
          common_objects_factory) {
    auto crypto = std::make_shared<CryptoProviderImpl>(
        keypair, std::move(common_objects_factory));

    return crypto;
  }

  auto createHashProvider() {
    return std::make_shared<YacHashProviderImpl>();
  }

  std::shared_ptr<Yac> createYac(
      ClusterOrdering initial_order,
      const shared_model::crypto::Keypair &keypair,
      std::shared_ptr<Timer> timer,
      std::shared_ptr<YacNetwork> network,
      std::shared_ptr<shared_model::interface::CommonObjectsFactory>
          common_objects_factory,
      ConsistencyModel consistency_model,
      const logger::LoggerManagerTreePtr &consensus_log_manager) {
    std::shared_ptr<iroha::consensus::yac::CleanupStrategy> cleanup_strategy =
        std::make_shared<iroha::consensus::yac::BufferedCleanupStrategy>();
    return Yac::create(
        YacVoteStorage(cleanup_strategy,
                       getSupermajorityChecker(consistency_model),
                       consensus_log_manager->getChild("VoteStorage")),
        std::move(network),
        createCryptoProvider(keypair, std::move(common_objects_factory)),
        std::move(timer),
        initial_order,
        consensus_log_manager->getChild("HashGate")->getLogger());
  }
}  // namespace

namespace iroha {
  namespace consensus {
    namespace yac {

      std::shared_ptr<NetworkImpl> YacInit::getConsensusNetwork() const {
        BOOST_ASSERT_MSG(initialized_,
                         "YacInit::initConsensusGate(...) must be called prior "
                         "to YacInit::getConsensusNetwork()!");
        return consensus_network_;
      }

      auto YacInit::createTimer(std::chrono::milliseconds delay_milliseconds) {
        return std::make_shared<TimerImpl>([delay_milliseconds, this] {
          // static factory with a single thread
          //
          // observe_on_new_thread -- coordination which creates new thread with
          // observe_on strategy -- all subsequent operations will be performed
          // on this thread.
          //
          // scheduler owns a timeline that is exposed by the now() method.
          // scheduler is also a factory for workers in that timeline.
          //
          // coordination is a factory for coordinators and has a scheduler.
          return rxcpp::observable<>::timer(
              std::chrono::milliseconds(delay_milliseconds), coordination_);
        });
      }

      std::shared_ptr<YacGate> YacInit::initConsensusGate(
          std::shared_ptr<iroha::ametsuchi::PeerQueryFactory>
              peer_query_factory,
          std::shared_ptr<simulator::BlockCreator> block_creator,
          std::shared_ptr<network::BlockLoader> block_loader,
          const shared_model::crypto::Keypair &keypair,
          std::shared_ptr<consensus::ConsensusResultCache>
              consensus_result_cache,
          std::chrono::milliseconds vote_delay_milliseconds,
          std::shared_ptr<
              iroha::network::AsyncGrpcClient<google::protobuf::Empty>>
              async_call,
          std::shared_ptr<shared_model::interface::CommonObjectsFactory>
              common_objects_factory,
          ConsistencyModel consistency_model,
          const logger::LoggerManagerTreePtr &consensus_log_manager) {
        auto peer_orderer = createPeerOrderer(peer_query_factory);

        consensus_network_ = std::make_shared<NetworkImpl>(
            async_call,
            [](const shared_model::interface::Peer &peer) {
              return network::createClient<proto::Yac>(peer.address());
            },
            consensus_log_manager->getChild("Network")->getLogger());

        auto yac = createYac(peer_orderer->getInitialOrdering().value(),
                             keypair,
                             createTimer(vote_delay_milliseconds),
                             consensus_network_,
                             std::move(common_objects_factory),
                             consistency_model,
                             consensus_log_manager);
        consensus_network_->subscribe(yac);

        auto hash_provider = createHashProvider();

        initialized_ = true;

        return std::make_shared<YacGateImpl>(
            std::move(yac),
            std::move(peer_orderer),
            hash_provider,
            block_creator,
            std::move(consensus_result_cache),
            consensus_log_manager->getChild("Gate")->getLogger());
      }
    }  // namespace yac
  }    // namespace consensus
}  // namespace iroha
