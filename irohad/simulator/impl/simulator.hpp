/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_SIMULATOR_HPP
#define IROHA_SIMULATOR_HPP

#include <boost/optional.hpp>

#include "ametsuchi/block_query_factory.hpp"
#include "ametsuchi/temporary_factory.hpp"
#include "cryptography/crypto_provider/crypto_model_signer.hpp"
#include "interfaces/iroha_internal/unsafe_block_factory.hpp"
#include "logger/logger.hpp"
#include "network/ordering_gate.hpp"
#include "simulator/block_creator.hpp"
#include "simulator/verified_proposal_creator.hpp"
#include "validation/stateful_validator.hpp"

namespace iroha {
  namespace simulator {

    class Simulator : public VerifiedProposalCreator, public BlockCreator {
     public:
      Simulator(
          std::shared_ptr<network::OrderingGate> ordering_gate,
          std::shared_ptr<validation::StatefulValidator> statefulValidator,
          std::shared_ptr<ametsuchi::TemporaryFactory> factory,
          std::shared_ptr<ametsuchi::BlockQueryFactory> block_query_factory,
          std::shared_ptr<shared_model::crypto::CryptoModelSigner<>>
              crypto_signer,
          std::unique_ptr<shared_model::interface::UnsafeBlockFactory>
              block_factory);

      ~Simulator() override;

      void process_proposal(
          const shared_model::interface::Proposal &proposal) override;

      rxcpp::observable<
          std::shared_ptr<iroha::validation::VerifiedProposalAndErrors>>
      on_verified_proposal() override;

      void process_verified_proposal(
          const shared_model::interface::Proposal &proposal) override;

      rxcpp::observable<std::shared_ptr<shared_model::interface::Block>>
      on_block() override;

     private:
      // internal
      rxcpp::subjects::subject<
          std::shared_ptr<iroha::validation::VerifiedProposalAndErrors>>
          notifier_;
      rxcpp::subjects::subject<std::shared_ptr<shared_model::interface::Block>>
          block_notifier_;

      rxcpp::composite_subscription proposal_subscription_;
      rxcpp::composite_subscription verified_proposal_subscription_;

      std::shared_ptr<validation::StatefulValidator> validator_;
      std::shared_ptr<ametsuchi::TemporaryFactory> ametsuchi_factory_;
      std::shared_ptr<ametsuchi::BlockQueryFactory> block_query_factory_;
      std::shared_ptr<shared_model::crypto::CryptoModelSigner<>> crypto_signer_;
      std::unique_ptr<shared_model::interface::UnsafeBlockFactory>
          block_factory_;

      logger::Logger log_;

      // last block
      std::shared_ptr<shared_model::interface::Block> last_block;
    };
  }  // namespace simulator
}  // namespace iroha

#endif  // IROHA_SIMULATOR_HPP
