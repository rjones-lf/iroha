/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "backend/protobuf/common_objects/proto_common_objects_factory.hpp"

#include <gtest/gtest.h>

using namespace shared_model;

TEST(PeerTest, ValidPeerInitialization) {
  proto::ProtoCommonObjectsFactory factory;

  auto peer = factory.createPeer("127.0.0.1", interface::types::PubkeyType(shared_model::crypto::Blob::fromHexString("1234")));

  ASSERT_EQ(peer->address(), "127.0.0.1");
  ASSERT_EQ(peer->pubkey().hex(), "1234");
}
