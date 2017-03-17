/*
Copyright Soramitsu Co., Ltd. 2016 All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

//
// Created by Takumi Yamashita on 2017/02/23.
//

#include <gtest/gtest.h>
#include <infra/protobuf/api.pb.h>
#include <service/peer_service.hpp>
#include <vector>


TEST(peer_service_with_json_test, initialize_peer_test) {
  peer::Nodes peers = ::peer::service::getPeerList();
  for (auto&& peer : peers) {
    std::cout << peer->getIP() << std::endl;
    std::cout << peer->getPublicKey() << std::endl;
    std::cout << peer->getTrustScore() << std::endl;
  }
  std::cout << "API:: address = " << Api::Peer::default_instance().address()
            << std::endl;
  std::cout << "API:: publicKey = " << Api::Peer::default_instance().publickey()
            << std::endl;
}

TEST(peer_service_with_json_test, add_peer_test) {
  std::size_t n = ::peer::service::getPeerList().size();
  peer::Node peer1 = peer::Node("ip_low", "publicKey1", 0.5);
  peer::Node peer2 = peer::Node("ip_high", "publicKey2", 1.5);
  peer::Node peer3 = peer::Node("ip_high", "publicKey1", 1.5);
  peer::Node peer4 = peer::Node("ip_4", "0_publicKey4", 100.0);
  ASSERT_TRUE(::peer::transaction::validator::add(peer1));
  ASSERT_TRUE(::peer::transaction::executor::add(peer1));
  ASSERT_TRUE(::peer::transaction::validator::add(peer2));
  ASSERT_TRUE(::peer::transaction::executor::add(peer2));
  ASSERT_FALSE(::peer::transaction::validator::add(peer3));
  ASSERT_FALSE(::peer::transaction::executor::add(peer3));
  ASSERT_TRUE(::peer::transaction::validator::add(peer4));
  ASSERT_TRUE(::peer::transaction::executor::add(peer4));
  peer::Nodes peers = ::peer::service::getPeerList();
  for (auto&& peer : peers) {
    std::cout << peer->getIP() << std::endl;
    std::cout << peer->getPublicKey() << std::endl;
    std::cout << peer->getTrustScore() << std::endl;
  }
  ASSERT_TRUE(peers.size() == n + 3);
}

TEST(peer_service_with_json_test, update_peer_test) {
  std::size_t n = ::peer::service::getPeerList().size();
  const std::string upd_ip = "updated_ip";
  const std::string upd_key = "publicKey1";
  const std::string upd_ng_key = "dummy";
  peer::Node peer = peer::Node(upd_ip, upd_key, -1.0);
  peer::Node peer4 = peer::Node("ip_4", "0_publicKey4", 100.0);
  peer::Node peer_ng = peer::Node(upd_ip, upd_ng_key, -1.0);
  ASSERT_TRUE(::peer::transaction::validator::update(upd_key, peer));
  ASSERT_TRUE(::peer::transaction::executor::update(upd_key, peer));
  ASSERT_FALSE(::peer::transaction::validator::update(upd_ng_key, peer));
  ASSERT_FALSE(::peer::transaction::executor::update(upd_ng_key, peer_ng));
  peer::Nodes peers = ::peer::service::getPeerList();
  for (auto&& peer : peers) {
    std::cout << peer->getIP() << std::endl;
    std::cout << peer->getPublicKey() << std::endl;
    std::cout << peer->getTrustScore() << std::endl;
    if (peer->getPublicKey() == upd_key) {
      ASSERT_TRUE(peer->getTrustScore() == -0.5);
      ASSERT_TRUE(peer->getIP() == upd_ip);
    }
  }
  ASSERT_TRUE(peers.size() == n);
}

TEST(peer_service_with_json_test, remove_peer_test) {
  std::size_t n = ::peer::service::getPeerList().size();
  const std::string rm_key = "publicKey1";
  ASSERT_TRUE(::peer::transaction::validator::remove(rm_key));
  ASSERT_TRUE(::peer::transaction::executor::remove(rm_key));
  ASSERT_FALSE(::peer::transaction::validator::remove(rm_key));
  ASSERT_FALSE(::peer::transaction::executor::remove(rm_key));
  peer::Nodes peers = ::peer::service::getPeerList();
  for (auto&& peer : peers) {
    std::cout << peer->getIP() << std::endl;
    std::cout << peer->getPublicKey() << std::endl;
    std::cout << peer->getTrustScore() << std::endl;
  }
  ASSERT_TRUE(peers.size() == n - 1);
}

TEST(peer_service_with_json_test, leader_peer_check_test) {
  peer::Nodes peers = ::peer::service::getPeerList();
  ASSERT_FALSE(::peer::myself::isLeader());
  for (auto&& peer : peers) {
    if (peer->getPublicKey() != ::peer::myself::getPublicKey()) {
      ASSERT_TRUE(::peer::transaction::executor::remove(peer->getPublicKey()));
    }
  }
  ASSERT_TRUE(::peer::myself::isLeader());
}
