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

TEST(ClusterOrderTest, ClusterOrderOnNext) {
  iroha::model::Peer p1;
  p1.address = "1";
  iroha::model::Peer p2;
  p2.address = "2";
  std::vector<iroha::model::Peer> peers = {p1, p2};
  auto order = iroha::consensus::yac::ClusterOrdering::create(peers);
  ASSERT_TRUE(order);
  ASSERT_EQ("1", order->currentLeader().address);
  ASSERT_EQ("2", order->switchToNext().currentLeader().address);
  ASSERT_EQ("1", order->switchToNext().currentLeader().address);
}
