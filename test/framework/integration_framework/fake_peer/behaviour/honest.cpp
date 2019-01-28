/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "framework/integration_framework/fake_peer/behaviour/honest.hpp"

#include <boost/algorithm/string/join.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include "backend/protobuf/proto_proposal_factory.hpp"
#include "backend/protobuf/transaction.hpp"
#include "common/result.hpp"
#include "framework/integration_framework/fake_peer/block_storage.hpp"
#include "framework/integration_framework/fake_peer/network/batches_for_round.hpp"
#include "framework/integration_framework/fake_peer/proposal_storage.hpp"
#include "interfaces/iroha_internal/transaction_batch.hpp"
#include "logger/logger.hpp"
#include "module/shared_model/builders/protobuf/proposal.hpp"
#include "validators/default_validator.hpp"

using namespace iroha::expected;

namespace integration_framework {
  namespace fake_peer {

    HonestBehaviour::HonestBehaviour()
        : proposal_factory_(
              std::make_unique<shared_model::proto::ProtoProposalFactory<
                  shared_model::validation::DefaultProposalValidator>>()) {}

    void HonestBehaviour::processYacMessage(YacMessageSPtr message) {
      getFakePeer().voteForTheSame(message);
    }

    std::string HonestBehaviour::getName() {
      return "honest behaviour";
    }

    LoaderBlockRequestResult HonestBehaviour::processLoaderBlockRequest(
        LoaderBlockRequest request) {
      const auto &block_storage = getFakePeer().getBlockStorage();
      if (!block_storage) {
        getLogger()->debug(
            "Got a Loader.retrieveBlock call, but have no block storage!");
        return {};
      }
      const auto block = block_storage->getBlockByHash(*request);
      if (!block) {
        getLogger()->debug(
            "Got a Loader.retrieveBlock call for {}, but have no such block!",
            request->toString());
        return {};
      }
      return block;
    }

    LoaderBlocksRequestResult HonestBehaviour::processLoaderBlocksRequest(
        LoaderBlocksRequest request) {
      const auto block_storage = getFakePeer().getBlockStorage();
      if (!block_storage) {
        getLogger()->debug(
            "Got a Loader.retrieveBlocks call, but have no block storage!");
        return {};
      }
      BlockStorage::HeightType current_height = request;
      BlockStorage::BlockSPtr block;
      LoaderBlocksRequestResult blocks;
      while ((block = block_storage->getBlockByHeight(current_height++))
             != nullptr) {
        blocks.emplace_back(block);
      }
      return blocks;
    }

    OrderingProposalRequestResult
    HonestBehaviour::processOrderingProposalRequest(
        const OrderingProposalRequest &request) {
      const auto proposal_storage = getFakePeer().getProposalStorage();
      if (!proposal_storage) {
        getLogger()->debug(
            "Got an OnDemandOrderingService.GetProposal call for round {}, "
            "but have no proposal storage! NOT returning a proposal.",
            request.toString());
        return boost::none;
      }
      auto opt_proposal = proposal_storage->getProposal(request);
      getLogger()->debug(
          "Got an OnDemandOrderingService.GetProposal call for round {}, "
          "{} returning a proposal.",
          request.toString(),
          opt_proposal ? "" : "NOT");
      return opt_proposal;
    }

    void HonestBehaviour::processOrderingBatches(
        const BatchesForRound &batches_for_round) {
      if (batches_for_round.batches.empty()) {
        getLogger()->debug(
            "Got an OnDemandOrderingService.SendBatches call with "
            "empty batches set. Ignoring it.");
        return;
      }
      auto &fake_peer = getFakePeer();
      auto proposal_storage = fake_peer.getProposalStorage();
      if (!proposal_storage) {
        getLogger()->debug(
            "Got an OnDemandOrderingService.SendBatches call, but have no "
            "proposal storage to store the incoming batches! Creating one.");
        fake_peer.setProposalStorage(std::make_shared<ProposalStorage>());
        proposal_storage = fake_peer.getProposalStorage();
        BOOST_ASSERT_MSG(proposal_storage,
                         "Failed to create a proposal storage!");
      }
      const auto &round = batches_for_round.round;
      const auto &batches = batches_for_round.batches;
      getLogger()->debug(
          "Got an OnDemandOrderingService.SendBatches call, storing the "
          "following batches for round {}: {}",
          round.toString(),
          boost::algorithm::join(
              batches | boost::adaptors::transformed([](const auto &batch) {
                return batch->toString();
              }),
              ",\n"));
      std::vector<shared_model::proto::Transaction> txs;
      auto opt_proposal = proposal_storage->getProposal(round);
      if (opt_proposal) {
        for (const auto &tx : (*opt_proposal)->getTransport().transactions()) {
          txs.emplace_back(tx);
        }
      }
      for (const auto &batch : batches) {
        for (const auto &tx : batch->transactions()) {
          txs.emplace_back(
              std::static_pointer_cast<shared_model::proto::Transaction>(tx)
                  ->getTransport());
        }
      }

      auto new_proposal_creation_result = proposal_factory_->createProposal(
          round.block_round, iroha::time::now(), txs);
      new_proposal_creation_result.match(
          [&proposal_storage,
           &round](ValueOf<decltype(new_proposal_creation_result)> &success) {
            std::shared_ptr<shared_model::interface::Proposal>
                new_proposal_iface = std::move(success.value);
            auto new_proposal_proto =
                std::static_pointer_cast<shared_model::proto::Proposal>(
                    std::move(new_proposal_iface));
            proposal_storage->storeProposal(round,
                                            std::move(new_proposal_proto));
          },
          [&,
           this](const ErrorOf<decltype(new_proposal_creation_result)> &error) {
            getLogger()->error(
                "Could not create a proposal for round {} "
                "with transactions {}: {}",
                round,
                logger::to_string(txs,
                                  [](const auto &tx) { return tx.toString(); }),
                error.error);
          });
    }

  }  // namespace fake_peer
}  // namespace integration_framework
