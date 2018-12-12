/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_GRPC_MOCKS_HPP
#define IROHA_GRPC_MOCKS_HPP

namespace testing {
  class MockServerWriter
      : public grpc::ServerWriterInterface<iroha::protocol::Block> {
    MOCK_METHOD1(Write, void(iroha::protocol::Block));
    MOCK_METHOD2(Write,
                 bool(const iroha::protocol::Block &, grpc::WriteOptions));
    MOCK_METHOD0(SendInitialMetadata, void());
    MOCK_METHOD1(NextMessageSize, bool(uint32_t *));
  };
}  // namespace testing

#endif  // IROHA_GRPC_MOCKS_HPP
