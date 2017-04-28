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
#include <main_generated.h>
#include <gtest/gtest.h>
#include <service/flatbuffer_service.h>
#include <membership_service/peer_service.hpp>
#include <utils/datetime.hpp>
#include <utils/expected.hpp>

#include <iostream>
#include <memory>
#include <unordered_map>

TEST(FlatbufferServiceTest, toString) {
  auto publicKey = "SamplePublicKey";
  // Build a request with the name set.
  flatbuffers::FlatBufferBuilder fbb;

  std::unique_ptr<std::vector<flatbuffers::Offset<flatbuffers::String>>>
    signatories(new std::vector<flatbuffers::Offset<flatbuffers::String>>());
  signatories->emplace_back(fbb.CreateString(publicKey));

  auto account_vec = [&] {
    flatbuffers::FlatBufferBuilder fbbAccount;

    std::unique_ptr<std::vector<flatbuffers::Offset<flatbuffers::String>>>
      signatories(new std::vector<flatbuffers::Offset<flatbuffers::String>>(
      {fbbAccount.CreateString("publicKey1")}));

    auto account = iroha::CreateAccountDirect(fbbAccount, publicKey, "alias",
                                              signatories.get(), 1);
    fbbAccount.Finish(account);

    std::unique_ptr<std::vector<uint8_t>> account_vec(
      new std::vector<uint8_t>());

    auto buf = fbbAccount.GetBufferPointer();

    account_vec->assign(buf, buf + fbbAccount.GetSize());

    return account_vec;
  }();

  auto command = iroha::CreateAccountAddDirect(fbb, account_vec.get());

  std::unique_ptr<std::vector<flatbuffers::Offset<iroha::Signature>>>
    signature_vec(new std::vector<flatbuffers::Offset<iroha::Signature>>());
  std::unique_ptr<std::vector<uint8_t>> signed_message(
    new std::vector<uint8_t>());
  signed_message->emplace_back('a');
  signed_message->emplace_back('b');
  signed_message->emplace_back('c');
  signed_message->emplace_back('d');

  signature_vec->emplace_back(iroha::CreateSignatureDirect(
    fbb, publicKey, signed_message.get(), 1234567));

  auto tx_offset = iroha::CreateTransactionDirect(
    fbb, publicKey, iroha::Command::AccountAdd, command.Union(),
    signature_vec.get(), nullptr, 0);
  fbb.Finish(tx_offset);
  auto tx = flatbuffers::BufferRef<iroha::Transaction>(fbb.GetBufferPointer(),
                                                       fbb.GetSize());

  std::cout << flatbuffer_service::toString(*tx.GetRoot()) << std::endl;
}

TEST(FlatbufferServicePeerTest, PeerServiceCreateAdd) {
  auto np = ::peer::Node("ip", "pubKey");
  flatbuffers::FlatBufferBuilder fbb;
  auto addPeer = flatbuffer_service::peer::CreateAdd(np);
  fbb.Finish(addPeer);

  auto addPeerPtr = flatbuffers::GetRoot<iroha::PeerAdd>(fbb.GetBufferPointer());

  auto peerRoot = addPeerPtr->peer_nested_root();
  ASSERT_STREQ(peerRoot->ip()->c_str(), "ip");
  ASSERT_STREQ(peerRoot->publicKey()->c_str(), "pubKey");
  ASSERT_STREQ(peerRoot->ip()->c_str(), np.ip.c_str());
  ASSERT_STREQ(peerRoot->publicKey()->c_str(), np.publicKey.c_str());
  ASSERT_EQ(peerRoot->trust(), np.trust);
  ASSERT_EQ(peerRoot->join_network(), np.join_network);
  ASSERT_EQ(peerRoot->join_validation(), np.join_validation);
}

TEST(FlatbufferServicePeerTest, PeerServiceCreateRemove) {
  flatbuffers::FlatBufferBuilder fbb;
  auto removePeer = flatbuffer_service::peer::CreateRemove("pubKey");
  fbb.Finish(removePeer);
  auto removePeerPtr = flatbuffers::GetRoot<iroha::PeerRemove>(fbb.GetBufferPointer());
  //ASSERT_EQ(removePeerPtr->command_type(), iroha::Command::PeerRemove);
  ASSERT_STREQ(removePeerPtr->peerPubKey()->c_str(), "pubKey");
}

TEST(FlatbufferServicePeerTest, PeerServiceCreateSetTrust) {
  flatbuffers::FlatBufferBuilder fbb;
  auto trust = 3.14159265;
  auto changeTrust = flatbuffer_service::peer::CreateChangeTrust("pubKey", trust);
  fbb.Finish(changeTrust);
  auto changeTrustPtr = flatbuffers::GetRoot<iroha::PeerChangeTrust>(fbb.GetBufferPointer());
  //ASSERT_EQ(changeTrustPtr->command_type(), ::iroha::Command::PeerChangeTrust);
  ASSERT_STREQ(changeTrustPtr->peerPubKey()->c_str(), "pubKey");
  ASSERT_EQ(changeTrustPtr->delta(), 3.14159265);
}

TEST(FlatbufferServicePeerTest, PeerServiceCreateChangeTrust) {
  flatbuffers::FlatBufferBuilder fbb;
  auto trust = 1.41421356;
  auto changeTrust = flatbuffer_service::peer::CreateChangeTrust("pubKey", trust);
  fbb.Finish(changeTrust);
  auto changeTrustPtr = flatbuffers::GetRoot<iroha::PeerChangeTrust>(fbb.GetBufferPointer());
  //ASSERT_EQ(changeTrustPtr->command_type(), ::iroha::Command::PeerChangeTrust);
  ASSERT_STREQ(changeTrustPtr->peerPubKey()->c_str(), "pubKey");
  ASSERT_EQ(changeTrustPtr->delta(), 1.41421356);
}

TEST(FlatbufferServicePeerTest, PeerServiceCreateSetActive) {
  flatbuffers::FlatBufferBuilder fbb;
  auto setActive = flatbuffer_service::peer::CreateSetActive("pubKey", true);
  fbb.Finish(setActive);
  auto setActivePtr = flatbuffers::GetRoot<iroha::PeerSetActive>(fbb.GetBufferPointer());
  ASSERT_STREQ(setActivePtr->peerPubKey()->c_str(), "pubKey");
  ASSERT_EQ(setActivePtr->active(), true);
}

TEST(FlatbufferServiceTest, PrimitivesCreatePeer) {
  ::peer::Node np("ip", "pubKey");
  auto peer = flatbuffer_service::primitives::CreatePeer(np);
  auto peerRoot = flatbuffers::GetRoot<::iroha::Peer>(peer.data());
  ASSERT_STREQ(peerRoot->ip()->c_str(), "ip");
  ASSERT_STREQ(peerRoot->publicKey()->c_str(), "pubKey");
  ASSERT_STREQ(peerRoot->ip()->c_str(), np.ip.c_str());
  ASSERT_STREQ(peerRoot->publicKey()->c_str(), np.publicKey.c_str());
  ASSERT_EQ(peerRoot->trust(), np.trust);
  ASSERT_EQ(peerRoot->join_network(), np.join_network);
  ASSERT_EQ(peerRoot->join_validation(), np.join_validation);
}

TEST(FlatbufferServiceTest, PrimitivesCreateSignature) {
  // TODO: NO IMPLEMENTATION
}

TEST(FlatbufferServiceTest, PrimitivesCreateTransaction) {

  flatbuffers::FlatBufferBuilder xbb;

  ::peer::Node np("ip", "pubKey");
  auto peer = flatbuffer_service::primitives::CreatePeer(np);
  auto peerAdd = iroha::CreatePeerAdd(xbb, xbb.CreateVector(peer));

  // TODO: Replace with primitives::CreateSignature()
  std::vector<flatbuffers::Offset<iroha::Signature>> signatures;

  std::vector<uint8_t> sig {'a','b','c','d'};
  auto sigoffset = iroha::CreateSignatureDirect(xbb, "PUBKEY", &sig, 1234567);
  signatures.push_back(sigoffset);

  auto txptr = flatbuffer_service::transaction::CreateTransaction(
    xbb,
    iroha::Command::PeerAdd,
    peerAdd.Union(),
    "Creator",
    signatures
  );

  ASSERT_STREQ(txptr->creatorPubKey()->c_str(), "Creator");
  ASSERT_EQ(txptr->command_type(), iroha::Command::PeerAdd);
  ASSERT_STREQ(txptr->signatures()->Get(0)->publicKey()->c_str(), "PUBKEY");
  ASSERT_EQ(txptr->signatures()->Get(0)->signature()->Get(0), 'a');
  ASSERT_EQ(txptr->signatures()->Get(0)->signature()->Get(1), 'b');
  ASSERT_EQ(txptr->signatures()->Get(0)->signature()->Get(2), 'c');
  ASSERT_EQ(txptr->signatures()->Get(0)->signature()->Get(3), 'd');
  ASSERT_EQ(txptr->signatures()->Get(0)->timestamp(), 1234567);
//  ASSERT_STREQ() // ハッシュ未テスト

  auto peerRoot = txptr->command_as_PeerAdd()->peer_nested_root();
  ASSERT_STREQ(peerRoot->ip()->c_str(), np.ip.c_str());
  ASSERT_STREQ(peerRoot->publicKey()->c_str(), np.publicKey.c_str());
  ASSERT_EQ(peerRoot->trust(), np.trust);
  ASSERT_EQ(peerRoot->join_network(), np.join_network);
  ASSERT_EQ(peerRoot->join_validation(), np.join_validation);
}
