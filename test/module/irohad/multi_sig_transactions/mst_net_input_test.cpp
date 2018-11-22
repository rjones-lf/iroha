/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>
#include "framework/common_constants.hpp"
#include "framework/integration_framework/integration_test_framework.hpp"
#include "mst.grpc.pb.h"

using namespace common_constants;
using namespace integration_framework;
using namespace iroha::network;

/**
 * @given ITF
 * @when it receives MstState that contains a transaction with a
 * command, where command type is not set
 * @then there should be no SEGFAULT
 */
TEST(MstNetInteraction, TypelessCommand) {
  bool enable_mst = true;
  IntegrationTestFramework itf(1, {}, true, enable_mst);
  itf.setInitialState(kAdminKeypair);
  auto internal_port = itf.internalPort();

  iroha::network::transport::MstState state;

  auto transaction = state.add_transactions();
  transaction->mutable_payload()->mutable_reduced_payload()->add_commands();

  grpc::ClientContext context;
  google::protobuf::Empty response;

  auto async_call =
      std::make_shared<AsyncGrpcClient<google::protobuf::Empty>>();

  std::string peer_address = "127.0.0.1:" + std::to_string(internal_port);

  std::unique_ptr<transport::MstTransportGrpc::StubInterface> client =
      transport::MstTransportGrpc::NewStub(grpc::CreateChannel(
          peer_address, grpc::InsecureChannelCredentials()));

  client->SendState(&context, state, &response);
}
