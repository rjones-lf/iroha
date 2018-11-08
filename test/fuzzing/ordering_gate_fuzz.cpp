/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <memory>
#include <libfuzzer/libfuzzer_macro.h>
#include "ordering/impl/on_demand_ordering_gate.hpp"
#include "module/irohad/ordering/ordering_mocks.hpp"
#include "module/shared_model/interface_mocks.hpp"
#include "backend/protobuf/proto_block_factory.hpp"
#include "validators/default_validator.hpp"

using namespace iroha::ordering;
using namespace testing;

struct OrderingGateFixture {
  std::shared_ptr<shared_model::proto::ProtoBlockFactory> block_factory_;
  std::shared_ptr<MockOnDemandOrderingService> ordering_service_;
  std::shared_ptr<transport::MockOdOsNotification> network_client_;

  rxcpp::subjects::subject<OnDemandOrderingGate::BlockRoundEventType> rounds_;
  MockUnsafeProposalFactory *proposal_factory_;
  std::shared_ptr<OnDemandOrderingGate> ordering_gate_;
  iroha::consensus::Round initial_round_ = {2, 1};

  OrderingGateFixture() :
          block_factory_(std::make_shared<shared_model::proto::ProtoBlockFactory>(std::make_unique<
                  shared_model::validation::DefaultUnsignedBlockValidator>())),
          ordering_service_(std::make_shared<MockOnDemandOrderingService>()),
          network_client_(std::make_shared<transport::MockOdOsNotification>()) {

    auto proposal_factory = std::make_unique<MockUnsafeProposalFactory>();
    proposal_factory_ = proposal_factory.get();
    ordering_gate_ = std::make_shared<OnDemandOrderingGate>(ordering_service_, network_client_,
                                                            rounds_.get_observable(),
                                                            std::move(proposal_factory),
                                                            initial_round_);

    // to suppress "uninteresting" gmock warnings
    EXPECT_CALL(*ordering_service_, onCollaborationOutcome(_)).Times(AtLeast(0));
    EXPECT_CALL(*network_client_, onRequestProposal(_)).Times(AtLeast(0));
    EXPECT_CALL(*proposal_factory_, unsafeCreateProposal(_, _, _)).Times(AtLeast(0));
  }
};

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, std::size_t size) {
  static OrderingGateFixture ordering_gate_fixture;
  if (size < 1) {
    return 0;
  }

  iroha::protocol::Block block;
  if (protobuf_mutator::libfuzzer::LoadProtoInput(true, data, size, &block)) {
    auto iroha_block = ordering_gate_fixture.block_factory_->createBlock(std::move(block));
    if (auto result = boost::get<iroha::expected::Value<std::unique_ptr<shared_model::interface::Block>>>(
            &iroha_block)) {
      ordering_gate_fixture.rounds_.get_subscriber().on_next(std::move(result->value));
    }
  }

  return 0;
}
