/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <chrono>
#include <thread>

#include <gmock/gmock.h>
#include "backend/protobuf/block.hpp"
#include "builders/protobuf/transaction.hpp"
#include "framework/integration_framework/integration_test_framework.hpp"
#include "framework/integration_framework/iroha_instance.hpp"
#include "framework/integration_framework/test_mstnet_irohad.hpp"
#include "module/irohad/multi_sig_transactions/mst_mocks.hpp"
#include "module/shared_model/builders/protobuf/block.hpp"

using namespace std::chrono_literals;
using namespace integration_framework;

using ::testing::_;
using ::testing::AtLeast;

class MstNetwork : public testing::Test {
 public:
  MstNetwork() : block(initBlock()) {}

  shared_model::proto::Block initBlock() {
    shared_model::interface::RolePermissionSet all_perms{};
    for (size_t i = 0; i < all_perms.size(); ++i) {
      auto perm = static_cast<shared_model::interface::permissions::Role>(i);
      all_perms.set(perm);
    }
    auto genesis_tx =
        shared_model::proto::TransactionBuilder()
            .creatorAccountId(IntegrationTestFramework::kAdminId)
            .createdTime(iroha::time::now())
            .addPeer("0.0.0.0:50541", key.publicKey())
            .addPeer("0.0.0.0:50551",
                     shared_model::crypto::DefaultCryptoAlgorithmType::
                         generateKeypair()
                             .publicKey())
            .createRole(IntegrationTestFramework::kDefaultRole, all_perms)
            .createDomain(IntegrationTestFramework::kDefaultDomain,
                          IntegrationTestFramework::kDefaultRole)
            .createAccount(IntegrationTestFramework::kAdminName,
                           IntegrationTestFramework::kDefaultDomain,
                           key.publicKey())
            .createAsset(IntegrationTestFramework::kAssetName,
                         IntegrationTestFramework::kDefaultDomain,
                         1)
            .quorum(1)
            .build()
            .signAndAddSignature(key)
            .finish();
    auto genesis_block =
        shared_model::proto::BlockBuilder()
            .transactions(
                std::vector<shared_model::proto::Transaction>{genesis_tx})
            .height(1)
            .prevHash(shared_model::crypto::DefaultHashProvider::makeHash(
                shared_model::crypto::Blob("")))
            .createdTime(iroha::time::now())
            .build()
            .signAndAddSignature(key)
            .finish();
    return genesis_block;
  }

  void init(std::function<void(std::shared_ptr<MockTransportGrpc>)> mocker,
            bool mst_enable = false) {
    instance = std::make_unique<TestMstnetIrohad>(
        "/tmp/block_store",
        IrohaInstance::getPostgreCredsOrDefault({}),
        11501,
        50541,
        1,
        1h,
        0ms,
        key,
        mst_enable,
        mocker);
  }

  void insertBlock() {
    instance->storage->reset();
    instance->resetOrderingService();
    instance->storage->insertBlock(block);
  }

  auto getTx() const {
    return shared_model::proto::TransactionBuilder()
        .creatorAccountId(IntegrationTestFramework::kAdminId)
        .createdTime(iroha::time::now())
        .addAssetQuantity(kAssetId, "1.0")
        .quorum(2)
        .build()
        .signAndAddSignature(key)
        .finish();
  }

  shared_model::crypto::Keypair key =
      shared_model::crypto::DefaultCryptoAlgorithmType::generateKeypair();
  shared_model::proto::Block block;
  const shared_model::interface::types::AssetIdType kAssetId =
      IntegrationTestFramework::kAssetName + "#"
      + IntegrationTestFramework::kDefaultDomain;
  std::unique_ptr<TestMstnetIrohad> instance;
};

TEST_F(MstNetwork, NoPropagationOnDisabled) {
  init([](auto mock) {
    // shouldn't be called
  });
  insertBlock();

  auto tx = getTx();
  instance->init();
  instance->getCommandServiceTransport()->Torii(
      nullptr, &tx.getTransport(), nullptr);
  std::this_thread::sleep_for(2s);
}

TEST_F(MstNetwork, PropagationOnEnabled) {
  init([](auto mock) { EXPECT_CALL(*mock, sendState(_, _)).Times(AtLeast(1)); },
       true);
  insertBlock();

  auto tx = getTx();
  instance->init();
  instance->getCommandServiceTransport()->Torii(
      nullptr, &tx.getTransport(), nullptr);
  std::this_thread::sleep_for(2s);
}
