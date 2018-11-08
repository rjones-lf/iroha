/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "block_loader_fixture.hpp"

class MockServerWriter
    : public grpc::ServerWriterInterface<iroha::protocol::Block> {
 public:
  MOCK_METHOD0(SendInitialMetadata, void(void));
  MOCK_METHOD2(Write,
               bool(const iroha::protocol::Block &msg, grpc::WriteOptions));
};

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, std::size_t size) {
  static fuzzing::BlockLoaderFixture fixture;

  if (size < 1) {
    return 0;
  }

  iroha::network::proto::BlocksRequest request;
  if (protobuf_mutator::libfuzzer::LoadProtoInput(true, data, size, &request)) {
    grpc::ServerContext context;
    MockServerWriter writer;
    fixture.block_loader_service_->retrieveBlocks(&context, &request, nullptr);
  }

  return 0;
}
