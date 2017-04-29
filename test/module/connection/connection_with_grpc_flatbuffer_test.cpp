/*
 * Copyright Soramitsu Co., Ltd. 2016 All Rights Reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *      http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <thread>
#include <unordered_map>

#include <crypto/signature.hpp>
#include <main_generated.h>
#include <service/flatbuffer_service.h>
#include <infra/config/peer_service_with_json.hpp>
#include <connection/connection.hpp>

using ConsensusEvent = iroha::ConsensusEvent;
using Transaction = iroha::Transaction;

const std::string verifyTxCreatorPubKey = "creator";

const std::string verifySenderPubKey = "karin";
const std::string verifyDomainName = "name";
const std::string verifyDomainOwnerPubKey = "pubkey1";

const std::string toriiSenderPubKey = "sate";
const std::string toriiPeerPubKey = "light";
const std::string toriiPeerAddress = "test_ip";

class connection_with_grpc_flatbuffer_test : public testing::Test {
 protected:
  void serverVerifyReceive() {
    connection::iroha::SumeragiImpl::Verify::receive(
        [](const std::string &from, flatbuffers::unique_ptr_t&& eventUniqPtr) {
          auto eventptr = flatbuffers::GetRoot<ConsensusEvent>(eventUniqPtr.get());
          std::cout << flatbuffer_service::toString(
                           *eventptr->transactions()->Get(0)->tx_nested_root())
                    << std::endl;
          /*
          ASSERT_EQ(event.transaction().senderpubkey(), verifySenderPubKey);
          ASSERT_EQ(event.transaction().domain().name(), verifyDomainName);
          ASSERT_EQ(event.transaction().domain().ownerpublickey(),
                    verifyDomainOwnerPubKey);
                    */
          exit(0);
        });
    connection::run();
  }

  void serverToriiReceive() {
    connection::iroha::SumeragiImpl::Torii::receive(
        [](const std::string &from, flatbuffers::unique_ptr_t&& transaction) {
          /*
          ASSERT_EQ(transaction.senderpubkey(), toriiSenderPubKey);
          ASSERT_EQ(transaction.peer().publickey(), toriiPeerPubKey);
          ASSERT_EQ(transaction.peer().address(), toriiPeerAddress);
          ASSERT_TRUE(transaction.peer().trust().value() == 1.0);
           */
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
        std::thread(&connection_with_grpc_flatbuffer_test::serverVerifyReceive, this);
    server_thread_torii =
        std::thread(&connection_with_grpc_flatbuffer_test::serverToriiReceive, this);
  }

  virtual void TearDown() {
    server_thread_verify.detach();
    server_thread_torii.detach();
  }
};

TEST_F(connection_with_grpc_flatbuffer_test, Transaction_Add_Asset) {

  flatbuffers::FlatBufferBuilder xbb;

  const auto assetBuf = []{
    flatbuffers::FlatBufferBuilder fbb;
    auto currency = iroha::CreateCurrencyDirect(fbb, "IROHA", "Domain", "Ledger", "description", 100);
    auto asset = iroha::CreateAsset(fbb, ::iroha::AnyAsset::Currency, currency.Union());
    fbb.Finish(asset);
    auto buf = fbb.GetBufferPointer();
    return std::vector<uint8_t>(buf, buf + fbb.GetSize());
  }();

  auto assetAdd = iroha::CreateAssetAddDirect(xbb, verifySenderPubKey.c_str(), &assetBuf);

  std::vector<flatbuffers::Offset<iroha::Signature>> signatures;
  std::vector<uint8_t> blob = {'a','b','c','d'};
  signatures.push_back(iroha::CreateSignatureDirect(xbb, "SIG'S PUBKEY", &blob, 9999));

  std::vector<uint8_t> hash = {'H','S'};
  std::vector<uint8_t> data = {'D','T'};
  auto attachment = iroha::CreateAttachmentDirect(xbb, "MIME", &data);
  auto txoffset = iroha::CreateTransactionDirect(xbb, verifyTxCreatorPubKey.c_str(),
                                                 iroha::Command::AssetAdd, assetAdd.Union(),
                                                 &signatures, &hash, attachment);
  xbb.Finish(txoffset);

  auto txflatbuf = xbb.ReleaseBufferPointer();
  auto txptr = flatbuffers::GetRoot<Transaction>(txflatbuf.get());
  auto event = flatbuffer_service::toConsensusEvent(*txptr);
  ASSERT_TRUE(event);
  flatbuffers::unique_ptr_t uptr;
  event.move_value(uptr);
  auto eventptr = flatbuffers::GetRoot<ConsensusEvent>(uptr.get());
  connection::iroha::SumeragiImpl::Verify::send(config::PeerServiceConfig::getInstance().getMyIp(), *eventptr);
}

TEST_F(connection_with_grpc_flatbuffer_test, Transaction_Add_Peer) {
  /*
  auto tx =
      TransactionBuilder<Add<Peer>>()
          .setSenderPublicKey(toriiSenderPubKey)
          .setPeer(txbuilder::createPeer(toriiPeerPubKey, toriiPeerAddress,
                                         txbuilder::createTrust(1.0, true)))
          .build();

  connection::iroha::PeerService::Sumeragi::send(::peer::myself::getIp(), tx);
   */
}