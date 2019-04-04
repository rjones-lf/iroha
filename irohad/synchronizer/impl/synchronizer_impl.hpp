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
       * Iterate through the peers which signed the commit message, load and
       * apply the missing blocks
       * @param msg - the commit message that triggered synchronization
       * @param top_block_height - the top block height of a peer that needs to
       * be synchronized
       * @param alternative_outcome - that kind of outcome will be propagated to
       * subscribers when block store height after synchronization is less than
       * expected
       */
      boost::optional<SynchronizationEvent> downloadMissingBlocks(
          const consensus::Synchronizable &msg,
          const shared_model::interface::types::HeightType top_block_height,
          const SynchronizationOutcomeType alternative_outcome);

      void processNext(const consensus::PairValid &msg);

      /**
       * Performs synchronization on rejects
       * @param msg - consensus gate message with a list of peers and a round
       * @param alternative_outcome - synchronization outcome when block store
       * height is equal to expected height after synchronization
       */
      void processDifferent(const consensus::Synchronizable &msg,
                            SynchronizationOutcomeType alternative_outcome);

      boost::optional<shared_model::interface::types::HeightType>
      getTopBlockHeight() const;

      boost::optional<std::unique_ptr<ametsuchi::MutableStorage>> getStorage();

      std::shared_ptr<validation::ChainValidator> validator_;
      std::shared_ptr<ametsuchi::MutableFactory> mutable_factory_;
      std::shared_ptr<ametsuchi::BlockQueryFactory> block_query_factory_;
      std::shared_ptr<network::BlockLoader> block_loader_;

      // internal
      rxcpp::composite_subscription notifier_lifetime_;
      rxcpp::subjects::subject<SynchronizationEvent> notifier_;
      rxcpp::composite_subscription subscription_;

      logger::LoggerPtr log_;
    };

  }  // namespace synchronizer
}  // namespace iroha

#endif  // IROHA_SYNCHRONIZER_IMPL_HPP
