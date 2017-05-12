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

#include <asset_generated.h>
#include <flatbuffers/flatbuffers.h>
#include <main_generated.h>
#include <primitives_generated.h>
#include <membership_service/peer_service.hpp>
#include <service/flatbuffer_service.h>
#include <crypto/signature.hpp>
#include <infra/config/peer_service_with_json.hpp>
#include <service/connection.hpp>

#include <iostream>
#include <string>
#include <thread>
#include <unordered_map>

using ConsensusEvent = iroha::ConsensusEvent;
using Transaction = iroha::Transaction;

class connection_with_grpc_flatbuffer_test : public testing::Test {
 protected:
  void serverVerifyReceive() {
    connection::iroha::SumeragiImpl::Verify::receive(
        [](const std::string& from, flatbuffers::unique_ptr_t&& eventUniqPtr) {
          auto eventptr =
              flatbuffers::GetRoot<ConsensusEvent>(eventUniqPtr.get());
          std::cout << flatbuffer_service::toString(
                           *eventptr->transactions()->Get(0)->tx_nested_root())
                    << std::endl;
          auto tx = eventptr->transactions()->Get(0)->tx_nested_root();
          ASSERT_STREQ(tx->creatorPubKey()->c_str(), "TX'S CREATOR");
          ASSERT_EQ(tx->hash()->Get(0), 'H');
          ASSERT_EQ(tx->hash()->Get(1), 'S');
          ASSERT_STREQ(tx->attachment()->mime()->c_str(), "MIME");
          ASSERT_EQ(tx->attachment()->data()->Get(0), 'D');
          ASSERT_EQ(tx->attachment()->data()->Get(1), 'T');
          // ASSERT_EQ(tx->command_type(), ::iroha::Command::AssetAdd);
          ASSERT_STREQ(tx->signatures()->Get(0)->publicKey()->c_str(),
                       "SIG'S PUBKEY");
          ASSERT_EQ(tx->signatures()->Get(0)->signature()->Get(0), 'a');
          ASSERT_EQ(tx->signatures()->Get(0)->signature()->Get(1), 'b');
          ASSERT_EQ(tx->signatures()->Get(0)->signature()->Get(2), 'c');
          ASSERT_EQ(tx->signatures()->Get(0)->signature()->Get(3), 'd');
          ASSERT_EQ(tx->signatures()->Get(0)->timestamp(), 9999);

          // ASSERT_STREQ(tx->command_as_AssetAdd()->accPubKey()->c_str(),
          // verifySenderPubKey.c_str());
          /*
                    auto currency =
             tx->command_as_AssetAdd()->asset_nested_root()->asset_as_Currency();
                    ASSERT_STREQ(currency->currency_name()->c_str(), "IROHA");
                    ASSERT_STREQ(currency->domain_name()->c_str(), "Domain");
                    ASSERT_STREQ(currency->ledger_name()->c_str(), "Ledger");
                    ASSERT_STREQ(currency->description()->c_str(),
             "description"); ASSERT_EQ(currency->amount(), 31415);
                    ASSERT_EQ(currency->precision(), 4);
                    */
        });
    connection::run();
  }

  void serverToriiReceive() {
    connection::iroha::SumeragiImpl::Torii::receive(
        [](const std::string& from, flatbuffers::unique_ptr_t&& transaction) {
          auto txRoot =
              flatbuffers::GetRoot<::iroha::Transaction>(transaction.get());
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

  std::thread server_thread_verify;
  std::thread server_thread_torii;

  static void SetUpTestCase() { connection::initialize_peer(); }

  static void TearDownTestCase() { connection::finish(); }

  virtual void SetUp() {
    logger::setLogLevel(logger::LogLevel::Debug);
    server_thread_verify = std::thread(
        &connection_with_grpc_flatbuffer_test::serverVerifyReceive, this);
    server_thread_torii = std::thread(
        &connection_with_grpc_flatbuffer_test::serverToriiReceive, this);
  }

  virtual void TearDown() {
    server_thread_verify.detach();
    server_thread_torii.detach();
  }
};

TEST_F(connection_with_grpc_flatbuffer_test, Transaction_Add_Asset) {
  flatbuffers::FlatBufferBuilder xbb;

  const auto assetBuf = flatbuffer_service::asset::CreateCurrency(
      "IROHA", "Domain", "Ledger", "Desc", "31415", 4);

  const auto add = ::iroha::CreateAddDirect(xbb, "AccPubKey", &assetBuf);

  std::vector<flatbuffers::Offset<iroha::Signature>> signatures;
  std::vector<uint8_t> blob = {'a', 'b', 'c', 'd'};
  signatures.push_back(
      iroha::CreateSignatureDirect(xbb, "SIG'S PUBKEY", &blob, 9999));

  std::vector<uint8_t> hash = {'H', 'S'};
  std::vector<uint8_t> data = {'D', 'T'};

  const auto stamp = 9999999;
  const auto attachment = iroha::CreateAttachmentDirect(xbb, "MIME", &data);
  const auto txoffset = iroha::CreateTransactionDirect(
      xbb, "TX'S CREATOR", iroha::Command::Add,
      add.Union(), &signatures, &hash, stamp, attachment);
  xbb.Finish(txoffset);

  auto txflatbuf = xbb.ReleaseBufferPointer();
  auto txptr = flatbuffers::GetRoot<Transaction>(txflatbuf.get());

  auto event = flatbuffer_service::toConsensusEvent(*txptr);
  ASSERT_TRUE(event);
  flatbuffers::unique_ptr_t uptr;
  event.move_value(uptr);
  auto eventptr = flatbuffers::GetRoot<ConsensusEvent>(uptr.get());
  connection::iroha::SumeragiImpl::Verify::send(
      config::PeerServiceConfig::getInstance().getMyIp(), *eventptr);
}
