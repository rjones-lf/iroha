/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "main/impl/on_demand_ordering_init.hpp"

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
        std::chrono::milliseconds delay) {
      shared_model::interface::types::HashType dummy_hash(std::string(
          '0', shared_model::crypto::DefaultCryptoAlgorithmType::kHashLength));

      auto hashes = notifier.get_observable()
                        .flat_map([](auto commit) {
                          return commit.synced_blocks.map(
                              [](auto block) { return block->hash(); });
                        })
                        .start_with(dummy_hash, dummy_hash);

      auto latest_hashes = hashes.zip(hashes.skip(1), hashes.skip(2));

      auto peers =
          notifier.get_observable()
              .with_latest_from(latest_hashes)
              .map([this, peer_query_factory](auto &&t)
                       -> ordering::OnDemandConnectionManager::CurrentPeers {
                auto &commit = std::get<0>(t);
                auto &current_hashes = std::get<1>(t);

                auto obs = commit.synced_blocks.as_blocking();
                if (obs.count() == 0) {
                  current_reject_round_++;
                } else {
                  current_reject_round_ = 1;

                  // retrieve peer list from database
                  peer_query_factory->createPeerQuery() | [](auto &&query) {
                    return query->getLedgerPeers();
                  } | [this](auto &&peers) {
                    current_peers_ = std::move(peers);
                  };

                  auto generate_permutation = [&](auto round) {
                    auto &hash =
                        std::get<decltype(round)::round>(current_hashes);
                    log_->debug("Using hash: {}", hash.toString());
                    auto &permutation = permutations_[decltype(round)::round];

                    std::seed_seq seed(hash.blob().begin(), hash.blob().end());
                    std::default_random_engine gen(seed);

                    permutation.resize(current_peers_.size());
                    std::iota(permutation.begin(), permutation.end(), 0);

                    std::shuffle(permutation.begin(), permutation.end(), gen);
                  };

                  generate_permutation(RoundTypeEnum<kCurrentRound>{});
                  generate_permutation(RoundTypeEnum<kNextRound>{});
                  generate_permutation(RoundTypeEnum<kRoundAfterNext>{});
                }

                auto peer = [this](auto round, auto pos) {
                  auto &permutation = permutations_[round];
                  auto &peer =
                      current_peers_[permutation[pos % permutation.size()]];
                  log_->debug("Using peer: {}", peer->toString());
                  return peer;
                };

                return {{peer(kCurrentRound, current_reject_round_ + 2),
                         peer(kNextRound, 2),
                         peer(kRoundAfterNext, 1),
                         peer(kCurrentRound, current_reject_round_)}};
              });

      return std::make_shared<ordering::OnDemandConnectionManager>(
          createNotificationFactory(std::move(async_call), delay), peers);
    }

    auto OnDemandOrderingInit::createGate(
        std::shared_ptr<ordering::OnDemandOrderingService> ordering_service,
        std::shared_ptr<ordering::transport::OdOsNotification> network_client,
        std::unique_ptr<shared_model::interface::UnsafeProposalFactory>
            factory) {
      return std::make_shared<ordering::OnDemandOrderingGate>(
          std::move(ordering_service),
          std::move(network_client),
          notifier.get_observable().map(
              [](auto commit)
                  -> ordering::OnDemandOrderingGate::BlockRoundEventType {
                auto obs = commit.synced_blocks.as_blocking();
                if (obs.count() == 0) {
                  return ordering::OnDemandOrderingGate::EmptyEvent{};
                } else {
                  return obs.last();
                }
              }),
          std::move(factory));
    }

    auto OnDemandOrderingInit::createService(size_t max_size) {
      return std::make_shared<ordering::OnDemandOrderingServiceImpl>(max_size);
    }

    std::shared_ptr<iroha::network::OrderingGate>
    OnDemandOrderingInit::initOrderingGate(
        size_t max_size,
        std::chrono::milliseconds delay,
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
        std::unique_ptr<shared_model::interface::UnsafeProposalFactory>
            factory) {
      auto ordering_service = createService(max_size);
      service = std::make_shared<ordering::transport::OnDemandOsServerGrpc>(
          ordering_service,
          std::move(transaction_factory),
          std::move(batch_parser),
          std::move(transaction_batch_factory));
      return createGate(
          ordering_service,
          createConnectionManager(
              std::move(peer_query_factory), std::move(async_call), delay),
          std::move(factory));
    }

  }  // namespace network
}  // namespace iroha
