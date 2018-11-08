/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "main/impl/on_demand_ordering_init.hpp"

#include <random>

#include "common/bind.hpp"
#include "cryptography/crypto_provider/crypto_defaults.hpp"
#include "datetime/time.hpp"
#include "interfaces/common_objects/peer.hpp"
#include "interfaces/common_objects/types.hpp"
#include "ordering/impl/on_demand_connection_manager.hpp"
#include "ordering/impl/on_demand_ordering_gate.hpp"
#include "ordering/impl/on_demand_ordering_service_impl.hpp"
#include "ordering/impl/on_demand_os_client_grpc.hpp"
#include "ordering/impl/on_demand_os_server_grpc.hpp"

namespace iroha {
  namespace network {

    auto OnDemandOrderingInit::createNotificationFactory(
        std::shared_ptr<network::AsyncGrpcClient<google::protobuf::Empty>>
            async_call,
        std::chrono::milliseconds delay) {
      return std::make_shared<ordering::transport::OnDemandOsClientGrpcFactory>(
          std::move(async_call),
          [] { return std::chrono::system_clock::now(); },
          delay);
    }

    auto OnDemandOrderingInit::createConnectionManager(
        std::shared_ptr<ametsuchi::PeerQueryFactory> peer_query_factory,
        std::shared_ptr<network::AsyncGrpcClient<google::protobuf::Empty>>
            async_call,
        std::chrono::milliseconds delay,
        std::vector<shared_model::interface::types::HashType> hashes) {
      // flat map hashes from committed blocks
      auto all_hashes = notifier.get_observable()
                            .flat_map([](auto commit) {
                              return commit.synced_blocks.map(
                                  [](auto block) { return block->hash(); });
                            })
                            // prepend hashes for the first two rounds
                            .start_with(hashes.at(0), hashes.at(1));

      // emit last 3 hashes
      auto latest_hashes =
          all_hashes.zip(all_hashes.skip(1), all_hashes.skip(2));

      auto map_peers =
          [this, peer_query_factory](
              auto &&t) -> ordering::OnDemandConnectionManager::CurrentPeers {
        auto &commit = std::get<0>(t);
        auto &current_hashes = std::get<1>(t);

        auto obs = commit.synced_blocks.as_blocking();
        // if no blocks were commited
        if (obs.count() == 0) {
          current_reject_round_++;
        } else {
          current_reject_round_ = 1;

          // retrieve peer list from database
          peer_query_factory->createPeerQuery() | [](auto &&query) {
            return query->getLedgerPeers();
          } | [this](auto &&peers) { current_peers_ = std::move(peers); };

          // generate permutation of peers list from corresponding round hash
          auto generate_permutation = [&](auto round) {
            auto &hash = std::get<round()>(current_hashes);
            log_->debug("Using hash: {}", hash.toString());
            auto &permutation = permutations_[round()];

            std::seed_seq seed(hash.blob().begin(), hash.blob().end());
            gen_.seed(seed);

            permutation.resize(current_peers_.size());
            std::iota(permutation.begin(), permutation.end(), 0);

            std::shuffle(permutation.begin(), permutation.end(), gen_);
          };

          generate_permutation(RoundTypeConstant<kCurrentRound>{});
          generate_permutation(RoundTypeConstant<kNextRound>{});
          generate_permutation(RoundTypeConstant<kRoundAfterNext>{});
        }

        auto peer = [this](auto round, auto pos) {
          auto &permutation = permutations_[round];
          // since reject round can be greater than number of peers, wrap it
          // with number of peers
          auto &peer = current_peers_[permutation[pos % permutation.size()]];
          log_->debug("Using peer: {}", peer->toString());
          return peer;
        };

        ordering::OnDemandConnectionManager::CurrentPeers peers;
        /*
         * See detailed description in
         * irohad/ordering/impl/on_demand_connection_manager.cpp
         *
         *   0 1 2
         * 0 o x v
         * 1 x v .
         * 2 v . .
         *
         * v, round 0 - kCurrentRoundRejectConsumer
         * v, round 1 - kNextRoundRejectConsumer
         * v, round 2 - kNextRoundCommitConsumer
         * o, round 0 - kIssuer
         */
        peers.peers.at(
            ordering::OnDemandConnectionManager::kCurrentRoundRejectConsumer) =
            peer(kCurrentRound, current_reject_round_ + 2);
        peers.peers.at(
            ordering::OnDemandConnectionManager::kNextRoundRejectConsumer) =
            peer(kNextRound, 2);
        peers.peers.at(
            ordering::OnDemandConnectionManager::kNextRoundCommitConsumer) =
            peer(kRoundAfterNext, 1);
        peers.peers.at(ordering::OnDemandConnectionManager::kIssuer) =
            peer(kCurrentRound, current_reject_round_);
        return peers;
      };

      auto peers = notifier.get_observable()
                       .with_latest_from(latest_hashes)
                       .map(map_peers);

      return std::make_shared<ordering::OnDemandConnectionManager>(
          createNotificationFactory(std::move(async_call), delay), peers);
    }

    auto OnDemandOrderingInit::createGate(
        std::shared_ptr<ordering::OnDemandOrderingService> ordering_service,
        std::shared_ptr<ordering::transport::OdOsNotification> network_client,
        std::shared_ptr<shared_model::interface::UnsafeProposalFactory> factory,
        consensus::Round initial_round) {
      return std::make_shared<ordering::OnDemandOrderingGate>(
          std::move(ordering_service),
          std::move(network_client),
          notifier.get_observable().map(
              [](auto commit)
                  -> ordering::OnDemandOrderingGate::BlockRoundEventType {
                auto obs = commit.synced_blocks.as_blocking();
                // if no blocks were commited
                if (obs.count() == 0) {
                  return ordering::OnDemandOrderingGate::EmptyEvent{};
                } else {
                  return obs.last();
                }
              }),
          std::move(factory),
          initial_round);
    }

    auto OnDemandOrderingInit::createService(
        size_t max_size,
        std::shared_ptr<shared_model::interface::UnsafeProposalFactory>
            factory) {
      return std::make_shared<ordering::OnDemandOrderingServiceImpl>(
          max_size, std::move(factory));
    }

    std::shared_ptr<iroha::network::OrderingGate>
    OnDemandOrderingInit::initOrderingGate(
        size_t max_size,
        std::chrono::milliseconds delay,
        std::vector<shared_model::interface::types::HashType> hashes,
        std::shared_ptr<ametsuchi::PeerQueryFactory> peer_query_factory,
        std::shared_ptr<
            ordering::transport::OnDemandOsServerGrpc::TransportFactoryType>
            transaction_factory,
        std::shared_ptr<shared_model::interface::TransactionBatchParser>
            batch_parser,
        std::shared_ptr<shared_model::interface::TransactionBatchFactory>
            transaction_batch_factory,
        std::shared_ptr<network::AsyncGrpcClient<google::protobuf::Empty>>
            async_call,
        std::shared_ptr<shared_model::interface::UnsafeProposalFactory> factory,
        consensus::Round initial_round) {
      auto ordering_service = createService(max_size, factory);
      service = std::make_shared<ordering::transport::OnDemandOsServerGrpc>(
          ordering_service,
          std::move(transaction_factory),
          std::move(batch_parser),
          std::move(transaction_batch_factory));
      return createGate(ordering_service,
                        createConnectionManager(std::move(peer_query_factory),
                                                std::move(async_call),
                                                delay,
                                                std::move(hashes)),
                        std::move(factory),
                        initial_round);
    }

  }  // namespace network
}  // namespace iroha
