/**
 * Copyright Soramitsu Co., Ltd. 2017 All Rights Reserved.
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

#include <grpc++/security/server_credentials.h>
#include <grpc++/server.h>
#include <grpc++/server_builder.h>
#include <gtest/gtest.h>

#include "framework/test_subscriber.hpp"
#include "module/irohad/ametsuchi/ametsuchi_mocks.hpp"
#include "module/irohad/model/model_mocks.hpp"

#include "network/impl/block_loader_impl.hpp"
#include "network/impl/block_loader_service.hpp"

using namespace iroha::network;
using namespace iroha::ametsuchi;
using namespace iroha::model;
using namespace framework::test_subscriber;

using testing::Return;
using testing::A;

class BlockLoaderTest : public testing::Test {
 public:
  void SetUp() override {
    peer = Peer();
    peer.address = "0.0.0.0:50051";
    peers.push_back(peer);
    peer_query = std::make_shared<MockPeerQuery>();
    storage = std::make_shared<MockBlockQuery>();
    provider = std::make_shared<MockCryptoProvider>();
    loader = std::make_shared<BlockLoaderImpl>(peer_query, storage, provider);
    service = std::make_shared<BlockLoaderService>(storage);

    grpc::ServerBuilder builder;
    int port = 0;
    builder.AddListeningPort(peer.address, grpc::InsecureServerCredentials(),
                             &port);
    builder.RegisterService(service.get());
    server = builder.BuildAndStart();
    ASSERT_TRUE(server);
    ASSERT_NE(port, 0);
  }

  Peer peer;
  std::vector<Peer> peers;
  std::shared_ptr<MockPeerQuery> peer_query;
  std::shared_ptr<MockBlockQuery> storage;
  std::shared_ptr<MockCryptoProvider> provider;
  std::shared_ptr<BlockLoaderImpl> loader;
  std::shared_ptr<BlockLoaderService> service;
  std::unique_ptr<grpc::Server> server;
};

TEST_F(BlockLoaderTest, ValidWhenSameTopBlock) {
  // Current block height 1 => Other block height 1 => no blocks received
  Block block;
  block.height = 1;

  EXPECT_CALL(*peer_query, getLedgerPeers()).WillOnce(Return(peers));
  EXPECT_CALL(*storage, getTopBlocks(1))
      .WillOnce(Return(rxcpp::observable<>::just(block)));
  EXPECT_CALL(*storage, getBlocksFrom(block.height + 1))
      .WillOnce(Return(rxcpp::observable<>::empty<Block>()));
  auto wrapper =
      make_test_subscriber<CallExact>(loader->retrieveBlocks(peer.pubkey), 0);
  wrapper.subscribe();

  ASSERT_TRUE(wrapper.validate());
}

TEST_F(BlockLoaderTest, ValidWhenOneBlock) {
  // Current block height 1 => Other block height 2 => one block received
  Block block;
  block.height = 1;

  Block top_block;
  block.height = block.height + 1;

  EXPECT_CALL(*provider, verify(A<const Block &>())).WillOnce(Return(true));
  EXPECT_CALL(*peer_query, getLedgerPeers()).WillOnce(Return(peers));
  EXPECT_CALL(*storage, getTopBlocks(1))
      .WillOnce(Return(rxcpp::observable<>::just(block)));
  EXPECT_CALL(*storage, getBlocksFrom(block.height + 1))
      .WillOnce(Return(rxcpp::observable<>::just(top_block)));
  auto wrapper =
      make_test_subscriber<CallExact>(loader->retrieveBlocks(peer.pubkey), 1);
  wrapper.subscribe(
      [&top_block](auto block) { ASSERT_EQ(block.height, top_block.height); });

  ASSERT_TRUE(wrapper.validate());
}

TEST_F(BlockLoaderTest, ValidWhenMultipleBlocks) {
  // Current block height 1 => Other block height n => n-1 blocks received
  Block block;
  block.height = 1;

  auto num_blocks = 2;
  auto next_height = block.height + 1;

  std::vector<Block> blocks;
  for (auto i = next_height; i < next_height + num_blocks; ++i) {
    Block block;
    block.height = i;
    blocks.push_back(block);
  }

  EXPECT_CALL(*provider, verify(A<const Block &>()))
      .Times(num_blocks)
      .WillRepeatedly(Return(true));
  EXPECT_CALL(*peer_query, getLedgerPeers()).WillOnce(Return(peers));
  EXPECT_CALL(*storage, getTopBlocks(1))
      .WillOnce(Return(rxcpp::observable<>::just(block)));
  EXPECT_CALL(*storage, getBlocksFrom(next_height))
      .WillOnce(Return(rxcpp::observable<>::iterate(blocks)));
  auto wrapper = make_test_subscriber<CallExact>(
      loader->retrieveBlocks(peer.pubkey), num_blocks);
  auto height = next_height;
  wrapper.subscribe(
      [&height](auto block) { ASSERT_EQ(block.height, height++); });

  ASSERT_TRUE(wrapper.validate());
}

TEST_F(BlockLoaderTest, ValidWhenBlockPresent) {
  // Request existing block => success
  Block requested_block;
  requested_block.hash = HashProviderImpl().get_hash(requested_block);

  EXPECT_CALL(*provider, verify(A<const Block &>())).WillOnce(Return(true));
  EXPECT_CALL(*peer_query, getLedgerPeers()).WillOnce(Return(peers));
  EXPECT_CALL(*storage, getBlocksFrom(1))
      .WillOnce(Return(rxcpp::observable<>::just(requested_block)));
  auto block = loader->retrieveBlock(peer.pubkey, requested_block.hash);

  ASSERT_TRUE(block.has_value());
  ASSERT_EQ(block.value(), requested_block);
}

TEST_F(BlockLoaderTest, ValidWhenBlockMissing) {
  // Request nonexisting block => failure
  Block present_block;
  present_block.hash = HashProviderImpl().get_hash(present_block);

  auto hash = present_block.hash;
  hash.fill(0);

  EXPECT_CALL(*peer_query, getLedgerPeers()).WillOnce(Return(peers));
  EXPECT_CALL(*storage, getBlocksFrom(1))
      .WillOnce(Return(rxcpp::observable<>::just(present_block)));
  auto block = loader->retrieveBlock(peer.pubkey, hash);

  ASSERT_FALSE(block.has_value());
}
