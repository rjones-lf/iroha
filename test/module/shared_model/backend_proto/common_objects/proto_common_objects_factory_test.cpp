/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "backend/protobuf/common_objects/proto_common_objects_factory.hpp"
#include "framework/result_fixture.hpp"

#include <gtest/gtest.h>

using namespace shared_model;
using namespace framework::expected;

TEST(PeerTest, ValidPeerInitialization) {
  proto::ProtoCommonObjectsFactory<int> factory;

  auto peer = factory.createPeer(
      "127.0.0.1",
      interface::types::PubkeyType(
          shared_model::crypto::Blob::fromHexString("1234")));

  peer.match(
      [](const ValueOf<decltype(peer)> &v) {
        ASSERT_EQ(v.value->address(), "127.0.0.1");
        ASSERT_EQ(v.value->pubkey().hex(), "1234");
      },
      [](const ErrorOf<decltype(peer)> &e) {
        FAIL();
      }
  );
}
