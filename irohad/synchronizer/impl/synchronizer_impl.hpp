/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_SYNCHRONIZER_IMPL_HPP
#define IROHA_SYNCHRONIZER_IMPL_HPP

#include "synchronizer/synchronizer.hpp"

#include "ametsuchi/mutable_factory.hpp"
#include "ametsuchi/peer_query_factory.hpp"
#include "logger/logger_fwd.hpp"
#include "network/block_loader.hpp"
#include "network/consensus_gate.hpp"
#include "validation/chain_validator.hpp"

namespace iroha {

  namespace ametsuchi {
    class BlockQueryFactory;
  }

  namespace synchronizer {

    class SynchronizerImpl : public Synchronizer {
     public:
      SynchronizerImpl(
          std::shared_ptr<network::ConsensusGate> consensus_gate,
          std::shared_ptr<validation::ChainValidator> validator,
          std::shared_ptr<ametsuchi::MutableFactory> mutable_factory,
          std::shared_ptr<ametsuchi::BlockQueryFactory> block_query_factory,
          std::shared_ptr<network::BlockLoader> block_loader,
          logger::LoggerPtr log);

      ~SynchronizerImpl() override;

      void processOutcome(consensus::GateObject object) override;
      rxcpp::observable<SynchronizationEvent> on_commit_chain() override;

     private:
      /**
       * Iterate through the peers which signed the commit_message, load and
       * apply the missing blocks
       * @param commit_message - the commit that triggered synchronization
       * @param storage - mutable storage to apply downloaded commits from other
       * peers
       * @param height - the top block height of a peer that needs to be
       * synchronized
       */
      boost::optional<SynchronizationEvent> downloadMissingBlocks(
          const consensus::VoteOther &msg,
          std::unique_ptr<ametsuchi::MutableStorage> storage,
          const shared_model::interface::types::HeightType height);

      void processNext(const consensus::PairValid &msg);
      void processDifferent(const consensus::VoteOther &msg);

      boost::optional<std::unique_ptr<ametsuchi::MutableStorage>> getStorage();

      std::shared_ptr<validation::ChainValidator> validator_;
      std::shared_ptr<ametsuchi::MutableFactory> mutable_factory_;
      std::shared_ptr<ametsuchi::BlockQueryFactory> block_query_factory_;
      std::shared_ptr<network::BlockLoader> block_loader_;

      // internal
      rxcpp::subjects::subject<SynchronizationEvent> notifier_;
      rxcpp::composite_subscription subscription_;

      logger::LoggerPtr log_;
    };

  }  // namespace synchronizer
}  // namespace iroha

#endif  // IROHA_SYNCHRONIZER_IMPL_HPP
