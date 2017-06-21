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
// Created by Takumi Yamashita on 2017/06/20.
//

#include <gtest/gtest.h>
#include <datetime/time.hpp>
#include <logger/logger.hpp>
#include <peer_service/change_state.hpp>
#include <peer_service/monitor.hpp>

TEST(PeerServiceMonitor, CheckPeerList) {
  static auto log = logger::Logger("PeerServiceCheckPeerList");

  peer_service::change_state::initialize();

  auto ps = peer_service::monitor::getAllPeerList();
  for (auto p : ps) {
    log.info(p->getIp());
  }
}

TEST(PeerServiceMonitor, AddPeer) {
  static auto log = logger::Logger("PeerServiceAddPeer");

  auto peer1 = peer_service::Node("ip_1", "pubkey_1", "Sian");
  ASSERT_TRUE(peer_service::change_state::validation::add(peer1));
  ASSERT_TRUE(peer_service::change_state::runtime::add(peer1));

  auto peer2 = peer_service::Node("ip_2", "pubkey_2", "Yoshihito");
  ASSERT_TRUE(peer_service::change_state::validation::add(peer2));
  ASSERT_TRUE(peer_service::change_state::runtime::add(peer2));

  auto peer3 = peer_service::Node("ip_3", "pubkey_3", "Akane");
  ASSERT_TRUE(peer_service::change_state::validation::add(peer3));
  ASSERT_TRUE(peer_service::change_state::runtime::add(peer3));

  auto ps = peer_service::monitor::getAllPeerList();
  for (auto p : ps) {
    log.info(p->getIp());
  }
  ASSERT_TRUE(ps.size() == 4);
}

TEST(PeerServiceMonitor, ActivePeer) {
  static auto log = logger::Logger("PeerServiceActivePeer");

  auto pub1 = "pubkey_1";
  auto pub2 = "pubkey_2";

  ASSERT_TRUE(peer_service::change_state::validation::setActive(
      pub1, peer_service::State::ACTIVE));
  ASSERT_TRUE(peer_service::change_state::runtime::setActive(
      pub1, peer_service::State::ACTIVE, iroha::time::now64()));

  ASSERT_TRUE(peer_service::change_state::validation::setActive(
      pub2, peer_service::State::ACTIVE));
  ASSERT_TRUE(peer_service::change_state::runtime::setActive(
      pub2, peer_service::State::ACTIVE, iroha::time::now64()));

  auto ps = peer_service::monitor::getAllPeerList();
  for (auto p : ps) {
    log.info(p->getIp() + " " + p->getPublicKey() + " " +
             std::to_string(p->getState()));
  }

  ASSERT_EQ(peer_service::monitor::getActivePeerSize(), 2);
  auto ap1 = peer_service::monitor::getActivePeerAt(0);
  auto ap2 = peer_service::monitor::getActivePeerAt(1);
  ASSERT_EQ(ap1->getPublicKey(), pub1);
  ASSERT_EQ(ap2->getPublicKey(), pub2);
}

TEST(PeerServiceMonitor, ChangeTrustPeer) {
  static auto log = logger::Logger("PeerServiceChangeTrustPeer");

  auto pub1 = "pubkey_1";
  auto pub2 = "pubkey_2";

  ASSERT_TRUE(peer_service::change_state::validation::setTrust(pub1, 50.0));
  ASSERT_TRUE(peer_service::change_state::runtime::setTrust(pub1, 50.0));

  ASSERT_EQ(peer_service::monitor::getActivePeerSize(), 2);

  auto ap1 = peer_service::monitor::getActivePeerAt(0);
  auto ap2 = peer_service::monitor::getActivePeerAt(1);

  ASSERT_EQ(ap1->getPublicKey(), pub2);
  ASSERT_EQ(ap2->getPublicKey(), pub1);

  ASSERT_EQ(ap1->getTrust(), 100.0);
  ASSERT_EQ(ap2->getTrust(), 50.0);

  ASSERT_TRUE(peer_service::change_state::validation::changeTrust(pub2, -60.0));
  ASSERT_TRUE(peer_service::change_state::runtime::changeTrust(pub2, -60.0));

  ASSERT_EQ(peer_service::monitor::getActivePeerSize(), 2);

  ap1 = peer_service::monitor::getActivePeerAt(0);
  ap2 = peer_service::monitor::getActivePeerAt(1);

  ASSERT_EQ(ap1->getPublicKey(), pub1);
  ASSERT_EQ(ap2->getPublicKey(), pub2);

  ASSERT_EQ(ap1->getTrust(), 50.0);
  ASSERT_EQ(ap2->getTrust(), 40.0);
}

TEST(PeerServiceMonitor, ErasePeer) {
  static auto log = logger::Logger("PeerServiceErasePeer");

  auto pub1 = "pubkey_1";
  ASSERT_FALSE(peer_service::change_state::validation::remove(pub1));

  ASSERT_TRUE(peer_service::change_state::validation::setActive(
      pub1, peer_service::State::PREPARE));
  ASSERT_TRUE(peer_service::change_state::runtime::setActive(
      pub1, peer_service::State::PREPARE));

  auto ps = peer_service::monitor::getActivePeerList();
  for (auto p : ps) {
    log.info(p->getIp() + " " + p->getPublicKey() + " " +
             std::to_string(p->getState()));
  }

  ASSERT_EQ(peer_service::monitor::getActivePeerSize(), 1);

  ASSERT_TRUE(peer_service::change_state::validation::remove(pub1));
  ASSERT_TRUE(peer_service::change_state::runtime::remove(pub1));

  ps = peer_service::monitor::getAllPeerList();
  for (auto p : ps) {
    log.info(p->getIp() + " " + p->getPublicKey() + " " +
             std::to_string(p->getState()));
  }
}