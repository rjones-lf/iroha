/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "block_loader_fixture.hpp"

namespace fuzzing {
  class MockServerWriter
      : public grpc::ServerWriterInterface<iroha::protocol::Block> {
    MOCK_METHOD1(Write, void(iroha::protocol::Block));
    MOCK_METHOD2(Write,
                 bool(const iroha::protocol::Block &, grpc::WriteOptions));
    MOCK_METHOD0(SendInitialMetadata, void());
    MOCK_METHOD1(NextMessageSize, bool(uint32_t *));
  };
}  // namespace fuzzing

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, std::size_t size) {
  static fuzzing::BlockLoaderFixture fixture;

  if (size < 1) {
    return 0;
  }

  iroha::network::proto::BlocksRequest request;
  if (protobuf_mutator::libfuzzer::LoadProtoInput(true, data, size, &request)) {
    grpc::ServerContext context;
    fuzzing::MockServerWriter serverWriter;
    fixture.block_loader_service_->retrieveBlocks(
        &context,
        &request,
        reinterpret_cast<grpc::ServerWriter<iroha::protocol::Block> *>(
            &serverWriter));
  }

  return 0;
}
