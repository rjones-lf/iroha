/**
 * Copyright Soramitsu Co., Ltd. 2016 All Rights Reserved.
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

//
// Created by Takumi Yamashita on 2017/04/25.
//

#include <gtest/gtest.h>
#include <membership_service/peer_service.hpp>


TEST(peer_service_test, initialize_peer_test) {
  peer::Nodes peers = ::peer::service::getAllPeerList();

  for (auto&& peer : peers) {
    std::cout << peer->ip << std::endl;
    std::cout << peer->publicKey << std::endl;
    std::cout << peer->trust << std::endl;
    std::cout << peer->active << std::endl;
    std::cout << peer->join_network << std::endl;
    std::cout << peer->join_validation << std::endl;
  }
  std::cout << "my address = " << ::peer::myself::getIp() << std::endl;
  std::cout << "my publicKey = " << ::peer::myself::getPublicKey() << std::endl;
  std::cout << "my privateKey = " << ::peer::myself::getPrivateKey() << std::endl;
}

TEST(peer_service_test, activate_peer_test) {
  auto peers = ::peer::service::getAllPeerList();
  auto n = peers.size();
  ASSERT_TRUE( ::peer::service::getActivePeerList().size() == 0 );
  for(auto&& peer : peers) {
    ASSERT_TRUE(::peer::transaction::validator::setActive(peer->publicKey,true));
    ASSERT_TRUE(::peer::transaction::executor::setActive(peer->publicKey,true));
  }
  peers = ::peer::service::getActivePeerList();
  ASSERT_TRUE( n == peers.size() );
}

TEST(peer_service_test, add_peer_test) {
  std::size_t n = ::peer::service::getActivePeerList().size();
  peer::Node peer1 = peer::Node("ip_low", "publicKey1", 5.0, true);
  peer::Node peer2 = peer::Node("ip_high", "publicKey2", 15.0, true);
  peer::Node peer3 = peer::Node("ip_high", "publicKey1", 15.0, true);
  peer::Node peer4 = peer::Node("ip_4", "0_publicKey4", 100.0, true);
  ASSERT_TRUE(::peer::transaction::validator::add(peer1));
  ASSERT_TRUE(::peer::transaction::executor::add(peer1));
  ASSERT_TRUE(::peer::transaction::validator::add(peer2));
  ASSERT_TRUE(::peer::transaction::executor::add(peer2));
  ASSERT_FALSE(::peer::transaction::validator::add(peer3));
  ASSERT_FALSE(::peer::transaction::executor::add(peer3));
  ASSERT_TRUE(::peer::transaction::validator::add(peer4));
  ASSERT_TRUE(::peer::transaction::executor::add(peer4));
  peer::Nodes peers = ::peer::service::getActivePeerList();
  for (auto&& peer : peers) {
    std::cout << peer->ip << std::endl;
    std::cout << peer->publicKey << std::endl;
    std::cout << peer->trust << std::endl;
    std::cout << peer->active << std::endl;
    std::cout << peer->join_network << std::endl;
    std::cout << peer->join_validation << std::endl;
  }
  ASSERT_TRUE(peers.size() == n + 3);
}

TEST(peer_service_test, set_and_change_peer_test) {
  std::size_t n = ::peer::service::getActivePeerList().size();
  const std::string upd_ip = "ip_low";
  const std::string upd_key = "publicKey1";
  const std::string upd_ng_key = "dummy";
  const double ch_trust = -1.0;
  const double set_trust = 100.0;
  ASSERT_TRUE(::peer::transaction::validator::setTrust(upd_key,set_trust));
  ASSERT_TRUE(::peer::transaction::executor::setTrust(upd_key,set_trust));
  ASSERT_FALSE(::peer::transaction::validator::setTrust(upd_ng_key,set_trust));
  ASSERT_FALSE(::peer::transaction::executor::setTrust(upd_ng_key,set_trust));
  ASSERT_TRUE(::peer::transaction::validator::changeTrust(upd_key,ch_trust));
  ASSERT_TRUE(::peer::transaction::executor::changeTrust(upd_key,ch_trust));
  ASSERT_TRUE(::peer::transaction::validator::setTrust(::peer::myself::getPublicKey(),1.0));
  ASSERT_TRUE(::peer::transaction::executor::setTrust(::peer::myself::getPublicKey(),1.0));
  peer::Nodes peers = ::peer::service::getActivePeerList();
  for (auto&& peer : peers) {
    std::cout << peer->ip << std::endl;
    std::cout << peer->publicKey << std::endl;
    std::cout << peer->trust << std::endl;
    if (peer->publicKey == upd_key) {
      ASSERT_TRUE(peer->trust == 99.0);
      ASSERT_TRUE(peer->ip == upd_ip);
    }
  }
  ASSERT_TRUE(peers.size() == n);
}


TEST(peer_service_test, remove_peer_test) {
  std::size_t n = ::peer::service::getActivePeerList().size();
  const std::string rm_key = "publicKey1";
  ASSERT_TRUE(::peer::transaction::validator::remove(rm_key));
  ASSERT_TRUE(::peer::transaction::executor::remove(rm_key));
  ASSERT_FALSE(::peer::transaction::validator::remove(rm_key));
  ASSERT_FALSE(::peer::transaction::executor::remove(rm_key));
  peer::Nodes peers = ::peer::service::getActivePeerList();
  for (auto&& peer : peers) {
    std::cout << peer->ip << std::endl;
    std::cout << peer->publicKey << std::endl;
    std::cout << peer->trust << std::endl;
    ASSERT_FALSE(peer->publicKey==rm_key);
  }
  ASSERT_TRUE(peers.size() == n - 1);
}

TEST(peer_service_test, leader_peer_check_test) {
  peer::Nodes peers = ::peer::service::getActivePeerList();
  ASSERT_FALSE(::peer::myself::isLeader());
  for (auto&& peer : peers) {
    if (peer->publicKey != ::peer::myself::getPublicKey()) {
      ASSERT_TRUE(::peer::transaction::executor::remove(peer->publicKey));
    }
  }
  ASSERT_TRUE(::peer::myself::isLeader());
}