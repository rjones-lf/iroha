/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ordering_service_fixture.hpp"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, std::size_t size) {
  static fuzzing::OrderingServiceFixture fixture;

  if (size < 1) {
    return 0;
  }

  std::shared_ptr<OnDemandOrderingServiceImpl> ordering_service_;
  std::shared_ptr<OnDemandOsServerGrpc> server_;

  auto proposal_factory = std::make_unique<MockUnsafeProposalFactory>();
  ordering_service_ = std::make_shared<OnDemandOrderingServiceImpl>(
      data[0], std::move(proposal_factory));
  server_ = std::make_shared<OnDemandOsServerGrpc>(
      ordering_service_,
      fixture.transaction_factory_,
      fixture.batch_parser_,
      fixture.transaction_batch_factory_);

  proto::BatchesRequest request;
  if (protobuf_mutator::libfuzzer::LoadProtoInput(
          true, data + 1, size - 1, &request)) {
    grpc::ServerContext context;
    google::protobuf::Empty response;
    server_->SendBatches(&context, &request, &response);
  }

  return 0;
}
