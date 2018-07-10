/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>

#include "backend/protobuf/common_objects/proto_common_objects_factory.hpp"
#include "cryptography/crypto_provider/crypto_defaults.hpp"
#include "framework/result_fixture.hpp"
#include "validators/field_validator.hpp"

using namespace shared_model;
using namespace framework::expected;

class PeerTest : public ::testing::Test {
 public:
  std::string valid_address = "127.0.0.1:8080";
  crypto::PublicKey valid_pubkey =
      crypto::DefaultCryptoAlgorithmType::generateKeypair().publicKey();
  std::string invalid_address = "127.0.0.1";
};

/**
 * @given valid data for peer
 * @when peer is created via factory
 * @then peer is successfully initialized
 */
TEST_F(PeerTest, ValidPeerInitialization) {
  proto::ProtoCommonObjectsFactory<validation::FieldValidator> factory;

  auto peer = factory.createPeer(valid_address, valid_pubkey);

  peer.match(
      [&](const ValueOf<decltype(peer)> &v) {
        ASSERT_EQ(v.value->address(), valid_address);
        ASSERT_EQ(v.value->pubkey().hex(), valid_pubkey.hex());
      },
      [](const ErrorOf<decltype(peer)> &e) { FAIL() << e.error; });
}

/**
 * @given invalid data for peer
 * @when peer is created via factory
 * @then peer is not initialized correctly
 */
TEST_F(PeerTest, InvalidPeerInitialization) {
  proto::ProtoCommonObjectsFactory<validation::FieldValidator> factory;

  auto keypair = crypto::DefaultCryptoAlgorithmType::generateKeypair();

  auto peer = factory.createPeer(invalid_address, keypair.publicKey());

  peer.match(
      [](const ValueOf<decltype(peer)> &v) { FAIL() << "Expected error case"; },
      [](const ErrorOf<decltype(peer)> &e) { SUCCEED(); });
}
