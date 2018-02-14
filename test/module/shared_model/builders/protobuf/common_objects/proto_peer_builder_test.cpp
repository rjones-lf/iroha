/**
 * Copyright Soramitsu Co., Ltd. 2018 All Rights Reserved.
 * http://soramitsu.co.jp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>

#include "builders/protobuf/common_objects/proto_peer_builder.hpp"

TEST(ProtoPeerBuilderTest, AddressFieldBuild) {
  shared_model::proto::PeerBuilder builder;

  auto expected_address = "127.0.0.1";
  auto peer = builder.address(expected_address).build();

  EXPECT_EQ(peer.address(), expected_address);
}

TEST(ProtoPeerBuilderTest, KeyFieldBuild) {
  shared_model::proto::PeerBuilder builder;

  auto expected_key = shared_model::crypto::PublicKey("very_secure_key");
  auto peer = builder.pubkey(expected_key).build();

  EXPECT_EQ(peer.pubkey(), expected_key);
}

TEST(ProtoPeerBuilderTest, AllFieldsBuild) {
  shared_model::proto::PeerBuilder builder;

  auto expected_address = "127.0.0.1";
  auto expected_key = shared_model::crypto::PublicKey("very_secure_key");
  auto peer = builder.address(expected_address).pubkey(expected_key).build();

  EXPECT_EQ(peer.address(), expected_address);
  EXPECT_EQ(peer.pubkey(), expected_key);
}
