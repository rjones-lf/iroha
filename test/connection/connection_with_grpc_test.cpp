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

#include <gtest/gtest.h>
#include <thread>
#include <unordered_map>

#include <consensus/connection/connection.hpp>
#include <infra/config/peer_service_with_json.hpp>
#include <service/peer_service.hpp>

#include <transaction_builder/transaction_builder.hpp>

using Api::ConsensusEvent;
using Api::Transaction;

using txbuilder::TransactionBuilder;
using type_signatures::Add;
using type_signatures::Domain;
using type_signatures::Account;
using type_signatures::Asset;
using type_signatures::SimpleAsset;
using type_signatures::Peer;

const std::string verifySenderPubKey = "karin";
const std::string verifyDomainName = "name";
const std::string verifyDomainOwnerPubKey = "pubkey1";

const std::string toriiSenderPubKey = "sate";
const std::string toriiPeerPubKey = "light";
const std::string toriiPeerAddress = "test_ip";

class connection_with_grpc_test : public testing::Test {
protected:
  void serverVerifyReceive() {
    connection::iroha::Sumeragi::Verify::receive(
        [](const std::string &from, ConsensusEvent &event) {
          std::cout << event.transaction().DebugString() << std::endl;
          ASSERT_EQ(event.transaction().senderpubkey(), verifySenderPubKey);
          ASSERT_EQ(event.transaction().domain().name(), verifyDomainName);
          ASSERT_EQ(event.transaction().domain().ownerpublickey(),
                    verifyDomainOwnerPubKey);
        });
    connection::run();
  }

  void serverToriiReceive() {
    connection::iroha::Sumeragi::Torii::receive(
        [](const std::string &from, Transaction &transaction) {
          ASSERT_EQ(transaction.senderpubkey(), toriiSenderPubKey);
          ASSERT_EQ(transaction.peer().publickey(), toriiPeerPubKey);
          ASSERT_EQ(transaction.peer().address(), toriiPeerAddress);
          ASSERT_TRUE(transaction.peer().trust().value() == 1.0);
        });
    connection::run();
  }

  std::thread server_thread_verify;
  std::thread server_thread_torii;

  static void SetUpTestCase() { connection::initialize_peer(); }

  static void TearDownTestCase() { connection::finish(); }

  virtual void SetUp() {
    logger::setLogLevel(logger::LogLevel::Debug);
    server_thread_verify =
        std::thread(&connection_with_grpc_test::serverVerifyReceive, this);
    server_thread_torii =
        std::thread(&connection_with_grpc_test::serverToriiReceive, this);
  }

  virtual void TearDown() {
    server_thread_verify.detach();
    server_thread_torii.detach();
  }
};

TEST_F(connection_with_grpc_test, Transaction_Add_Domain) {
  Api::Domain domain;
  domain.set_ownerpublickey(verifyDomainOwnerPubKey);
  domain.set_name(verifyDomainName);
  auto tx = TransactionBuilder<Add<Domain>>()
      .setSenderPublicKey(verifySenderPubKey)
      .setDomain(domain)
      .build();

  Api::ConsensusEvent sampleEvent;
  sampleEvent.mutable_transaction()->CopyFrom(tx);

  connection::iroha::Sumeragi::Verify::send(::peer::myself::getIp(),
                                            sampleEvent);
}

TEST_F(connection_with_grpc_test, Transaction_Add_Peer) {
  auto tx =
      TransactionBuilder<Add<Peer>>()
          .setSenderPublicKey(toriiSenderPubKey)
          .setPeer(txbuilder::createPeer(toriiPeerPubKey, toriiPeerAddress,
                                         txbuilder::createTrust(1.0, true)))
          .build();

  connection::iroha::PeerService::Sumeragi::send(::peer::myself::getIp(), tx);
}
