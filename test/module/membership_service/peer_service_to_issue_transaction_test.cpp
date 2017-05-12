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
// Created by Takumi Yamashita on 2017/05/02.
//

#include <gtest/gtest.h>
#include <service/flatbuffer_service.h>
#include <transaction_generated.h>
#include <membership_service/peer_service.hpp>
#include <service/connection.hpp>

#include <iostream>
#include <string>
#include <thread>
#include <unordered_map>
#include <commands_generated.h>

using Transaction = iroha::Transaction;

class peer_service_to_issue_transaction_test : public ::testing::Test {
 protected:
  void serverToriiReceive() {
    connection::iroha::SumeragiImpl::Torii::receive([](
        const std::string& from, flatbuffers::unique_ptr_t&& transaction) {
      auto tx = flatbuffers::GetRoot<::iroha::Transaction>(transaction.get());
      std::cout << "Command type: " << (int)tx->command_type() << std::endl;

      if (tx->command_type() == iroha::Command::PeerAdd) {
        std::cout << "Command Add!" << std::endl;
        auto peer_add = tx->command_as_PeerAdd();
        auto peer = peer_add->peer_nested_root();
        std::cout << peer->ip()->str() << std::endl;
        std::cout << peer->publicKey()->str() << std::endl;
        std::cout << peer->ledger_name()->str() << std::endl;
        std::cout << peer->trust() << std::endl;
        std::cout << peer->active() << std::endl;
        std::cout << peer->join_ledger() << std::endl;
      } else if(tx->command_type() == iroha::Command::PeerRemove) {
        std::cout << "Command Remove!" << std::endl;
        auto peer_remove = tx->command_as_PeerRemove();
        std::cout << peer_remove->peerPubKey()->str() << std::endl;
      } else if(tx->command_type() == iroha::Command::PeerSetTrust) {
        std::cout << "Command SetTrust!" << std::endl;;
        auto peer_set_trust = tx->command_as_PeerSetTrust();
        std::cout << peer_set_trust->peerPubKey()->str() << std::endl;
        std::cout << peer_set_trust->trust() << std::endl;
      } else if(tx->command_type() == iroha::Command::PeerChangeTrust) {
        std::cout << "Command ChangeTrust!" << std::endl;;
        auto peer_change_trust = tx->command_as_PeerChangeTrust();
        std::cout << peer_change_trust->peerPubKey()->str() << std::endl;
        std::cout << peer_change_trust->delta() << std::endl;
      } else if(tx->command_type() == iroha::Command::PeerSetActive) {
        std::cout << "Command SetActive!" << std::endl;
        auto peer_set_active = tx->command_as_PeerSetActive();
        std::cout << peer_set_active->peerPubKey()->str() << std::endl;
        std::cout << peer_set_active->active() << std::endl;
      }
      /*
       *
      ASSERT_EQ(transaction.senderpubkey(), toriiSenderPubKey);
      ASSERT_EQ(transaction.peer().publickey(), toriiPeerPubKey);
      ASSERT_EQ(transaction.peer().address(), toriiPeerAddress);
      ASSERT_TRUE(transaction.peer().trust().value() == 1.0);
       */
    });
    connection::run();
  }
  std::thread server_thread_torii;

  static void SetUpTestCase() { connection::initialize_peer(); }

  static void TearDownTestCase() { connection::finish(); }

  virtual void SetUp() {
    server_thread_torii = std::thread(
        &peer_service_to_issue_transaction_test::serverToriiReceive, this);
  }

  virtual void TearDown() { server_thread_torii.detach(); }
};


TEST_F(peer_service_to_issue_transaction_test, PeerAddTest) {
  ::peer::service::initialize();
  connection::wait_till_ready();
  ::peer::myself::activate();
  const auto peer = ::peer::Node("new_ip", "new_pubkey", "ledger");
  ::peer::transaction::isssue::add(::peer::myself::getIp(), peer);

  const auto pubkey = ::peer::myself::getPublicKey();
  ::peer::transaction::isssue::remove(::peer::myself::getIp(), pubkey);

  const auto trust = 5.0;
  ::peer::transaction::isssue::setTrust(::peer::myself::getIp(), pubkey, trust);

  ::peer::transaction::isssue::changeTrust(::peer::myself::getIp(), pubkey, trust);

  ::peer::transaction::isssue::setActive(::peer::myself::getIp(), pubkey, true);
}

