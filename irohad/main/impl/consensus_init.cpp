/**
 * Copyright Soramitsu Co., Ltd. 2018 All Rights Reserved.
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

#include "main/impl/consensus_init.hpp"
#include "consensus/yac/impl/peer_orderer_impl.hpp"
#include "consensus/yac/impl/timer_impl.hpp"
#include "consensus/yac/impl/yac_crypto_provider_impl.hpp"
#include "consensus/yac/impl/yac_gate_impl.hpp"
#include "consensus/yac/impl/yac_hash_provider_impl.hpp"
#include "consensus/yac/storage/yac_proposal_storage.hpp"
#include "consensus/yac/transport/impl/network_impl.hpp"

namespace iroha {
  namespace consensus {
    namespace yac {

      auto YacInit::createPeerOrderer(
          std::shared_ptr<ametsuchi::PeerQueryFactory> peer_query_factory) {
        return std::make_shared<PeerOrdererImpl>(peer_query_factory);
      }

      auto YacInit::createNetwork(
          std::shared_ptr<
              iroha::network::AsyncGrpcClient<google::protobuf::Empty>>
              async_call) {
        consensus_network = std::make_shared<NetworkImpl>(async_call);
        return consensus_network;
      }

      auto YacInit::createCryptoProvider(
          const shared_model::crypto::Keypair &keypair,
          std::shared_ptr<shared_model::interface::CommonObjectsFactory>
              common_objects_factory) {
        auto crypto = std::make_shared<CryptoProviderImpl>(
            keypair, std::move(common_objects_factory));

        return crypto;
      }

      auto YacInit::createTimer(std::chrono::milliseconds delay_milliseconds) {
        return std::make_shared<TimerImpl>([delay_milliseconds] {
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
          //
          // coordinator has a worker, and is a factory for coordinated
          // observables, subscribers and schedulable functions.
          //
          // A new thread scheduler is created
          // by calling .create_coordinator().get_scheduler()
          //
          // static allows to reuse the same thread in subsequent calls to this
          // lambda
          static rxcpp::observe_on_one_worker coordination(
              rxcpp::observe_on_new_thread()
                  .create_coordinator()
                  .get_scheduler());
          return rxcpp::observable<>::timer(
              std::chrono::milliseconds(delay_milliseconds), coordination);
        });
      }

      auto YacInit::createHashProvider() {
        return std::make_shared<YacHashProviderImpl>();
      }

      std::shared_ptr<consensus::yac::Yac> YacInit::createYac(
          ClusterOrdering initial_order,
          const shared_model::crypto::Keypair &keypair,
          std::chrono::milliseconds delay_milliseconds,
          std::shared_ptr<
              iroha::network::AsyncGrpcClient<google::protobuf::Empty>>
              async_call,
          std::shared_ptr<shared_model::interface::CommonObjectsFactory>
              common_objects_factory) {
        return Yac::create(
            YacVoteStorage(),
            createNetwork(std::move(async_call)),
            createCryptoProvider(keypair, std::move(common_objects_factory)),
            createTimer(delay_milliseconds),
            initial_order);
      }

      std::shared_ptr<YacGate> YacInit::initConsensusGate(
          std::shared_ptr<ametsuchi::PeerQueryFactory> peer_query_factory,
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
              common_objects_factory) {
        auto peer_orderer = createPeerOrderer(peer_query_factory);

        auto yac = createYac(peer_orderer->getInitialOrdering().value(),
                             keypair,
                             vote_delay_milliseconds,
                             std::move(async_call),
                             std::move(common_objects_factory));
        consensus_network->subscribe(yac);

        auto hash_provider = createHashProvider();
        return std::make_shared<YacGateImpl>(std::move(yac),
                                             std::move(peer_orderer),
                                             hash_provider,
                                             block_creator,
                                             block_loader,
                                             std::move(consensus_result_cache));
      }
    }  // namespace yac
  }    // namespace consensus
}  // namespace iroha
