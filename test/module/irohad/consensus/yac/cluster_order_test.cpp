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

#include "consensus/yac/cluster_order.hpp"
#include <gtest/gtest.h>
#include "module/irohad/consensus/yac/yac_mocks.hpp"

class ClusterOrderTest : public ::testing::Test {
 protected:
  void SetUp() override {
    p1 = iroha::consensus::yac::mk_peer("1");
    p2 = iroha::consensus::yac::mk_peer("2");
    peers_list = {p1, p2};
  }

  std::shared_ptr<shared_model::interface::Peer> p1;
  std::shared_ptr<shared_model::interface::Peer> p2;

  std::vector<std::shared_ptr<shared_model::interface::Peer>> peers_list;
  std::vector<std::shared_ptr<shared_model::interface::Peer>> empty_peers_list;
};

/**
 * @given nonempty peers list
 * @when cluster order is created
 * @then create function returns nonempty object
 */
TEST_F(ClusterOrderTest, GoodClusterOrderCreation) {
  auto order = iroha::consensus::yac::ClusterOrdering::create(peers_list);
  ASSERT_TRUE(order);
}

/**
 * @given empty peers list
 * @when cluster order is created
 * @then create function returns empty object
 */
TEST_F(ClusterOrderTest, BadClusterOrderCreation) {
  auto empty_order =
      iroha::consensus::yac::ClusterOrdering::create(empty_peers_list);
  ASSERT_FALSE(empty_order);
}

TEST_F(ClusterOrderTest, ClusterOrderOnNext) {
  auto order = iroha::consensus::yac::ClusterOrdering::create(peers_list);
  ASSERT_TRUE(order);
  ASSERT_EQ("1", order->currentLeader().address());
  ASSERT_EQ("2", order->switchToNext().currentLeader().address());
  ASSERT_EQ("1", order->switchToNext().currentLeader().address());
}
