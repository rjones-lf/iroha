/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef VENDOR_GRPC_MOCKS_HPP
#define VENDOR_GRPC_MOCKS_HPP

#include <gmock/gmock.h>
#include <grpc++/grpc++.h>

namespace iroha {
  class MockServerWriter
      : public grpc::ServerWriterInterface<iroha::protocol::ToriiResponse> {
   public:
    MOCK_METHOD1(Write, void(iroha::protocol::ToriiResponse));
    MOCK_METHOD2(Write,
                 bool(const iroha::protocol::ToriiResponse &,
                      grpc::WriteOptions));
    MOCK_METHOD0(SendInitialMetadata, void());
    MOCK_METHOD1(NextMessageSize, bool(uint32_t *));
  };
}  // namespace iroha

#endif  // VENDOR_GRPC_MOCKS_HPP
