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
#include <generated/main_generated.h>
#include <gtest/gtest.h>
#include <service/flatbuffer_service.h>
#include <iostream>
#include <membership_service/peer_service.hpp>
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
    fbb, publicKey, iroha::Command::Command_AccountAdd, command.Union(),
    signature_vec.get(), nullptr, 0);
  fbb.Finish(tx_offset);
  auto tx = flatbuffers::BufferRef<iroha::Transaction>(fbb.GetBufferPointer(),
                                                       fbb.GetSize());

  std::cout << flatbuffer_service::toString(*tx.GetRoot()) << std::endl;
}

TEST(FlatbufferServicePeerTest, PeerService) {
  auto np = ::peer::Node("ip", "pubKey");
  flatbuffers::FlatBufferBuilder fbb;
  auto addPeer = flatbuffer_service::peer::CreateAdd(np);
}

TEST(FlatbufferServiceTest, toConsensusEvent) {
  flatbuffers::FlatBufferBuilder fbb;

  const auto accountBuf = [&] {
    return flatbuffer_service::CreateAccountBuffer("PublicKey", "Alias\u30e6",
                                                   {"sig1", "sig2", "sig3"}, 1);
  }();

  const auto signatureOffsets = [&] {
    std::vector<uint8_t> sigblob1 = {'a', 'b'};
    std::vector<uint8_t> sigblob2 = {'\0', 'a', '\0', 'b'};
    return std::vector<flatbuffers::Offset<::iroha::Signature>>{
      ::iroha::CreateSignatureDirect(fbb, "TxPubKey1", &sigblob1, 100000),
      ::iroha::CreateSignatureDirect(fbb, "TxPubKey2", &sigblob2, 100001)};
  }();

  const auto _hash = std::vector<uint8_t>{'h', '\0', '?', '\0'};

  const auto attachmentOffset = [&] {
    auto data = std::vector<uint8_t>{'d', '\0', '!'};
    return ::iroha::CreateAttachmentDirect(
      fbb, "=?ISO-2022-JP?B?VG95YW1hX05hbw==?=", &data);
  }();

  const auto txOffset = ::iroha::CreateTransactionDirect(
    fbb, "Creator PubKey", iroha::Command_AccountAdd,
    ::iroha::CreateAccountAddDirect(fbb, &accountBuf).Union(),
    &signatureOffsets, &_hash, attachmentOffset);

  fbb.Finish(txOffset);

  const auto ptr = fbb.ReleaseBufferPointer();
  const auto txptr = flatbuffers::GetRoot<::iroha::Transaction>(ptr.get());

  auto consensusEvent = flatbuffer_service::toConsensusEvent(*txptr);
  ASSERT_TRUE(consensusEvent);

  flatbuffers::unique_ptr_t uptr;
  consensusEvent.move_value(uptr);

  const auto root = flatbuffers::GetRoot<::iroha::ConsensusEvent>(uptr.get());

  // validate peerSignatures()
  ASSERT_TRUE(root->peerSignatures()->size() == 0);

  // validate transactions()
  const auto txptrFromEvent =
    root->transactions()->Get(0);  // toConsensusEvent() receives 1 tx.

  ASSERT_STREQ(txptrFromEvent->creatorPubKey()->c_str(), "Creator PubKey");
  ASSERT_EQ(txptrFromEvent->command_type(), ::iroha::Command_AccountAdd);

  // validate nested account
  const auto accroot =
    txptrFromEvent->command_as_AccountAdd()->account_nested_root();
  ASSERT_STREQ(accroot->pubKey()->c_str(), "PublicKey");
  ASSERT_STREQ(accroot->alias()->c_str(), "Alias\u30e6");
  ASSERT_STREQ(accroot->signatories()->Get(0)->c_str(), "sig1");
  ASSERT_STREQ(accroot->signatories()->Get(1)->c_str(), "sig2");
  ASSERT_STREQ(accroot->signatories()->Get(2)->c_str(), "sig3");
  ASSERT_EQ(accroot->useKeys(), 1);

  // validate signatures
  ASSERT_STREQ(txptrFromEvent->signatures()->Get(0)->publicKey()->c_str(),
               "TxPubKey1");
  ASSERT_EQ(txptrFromEvent->signatures()->Get(0)->signature()->Get(0), 'a');
  ASSERT_EQ(txptrFromEvent->signatures()->Get(0)->signature()->Get(1), 'b');
  ASSERT_EQ(txptrFromEvent->signatures()->Get(0)->signature()->size(), 2);
  ASSERT_EQ(txptrFromEvent->signatures()->Get(0)->timestamp(), 100000);

  ASSERT_EQ(txptrFromEvent->signatures()->Get(1)->signature()->Get(0), '\0');
  ASSERT_EQ(txptrFromEvent->signatures()->Get(1)->signature()->Get(1), 'a');
  ASSERT_EQ(txptrFromEvent->signatures()->Get(1)->signature()->Get(2), '\0');
  ASSERT_EQ(txptrFromEvent->signatures()->Get(1)->signature()->Get(3), 'b');
  ASSERT_EQ(txptrFromEvent->signatures()->Get(1)->signature()->size(), 4);
  ASSERT_EQ(txptrFromEvent->signatures()->Get(1)->timestamp(), 100001);

  ASSERT_EQ(txptrFromEvent->hash()->Get(0), 'h');
  ASSERT_EQ(txptrFromEvent->hash()->Get(1), '\0');
  ASSERT_EQ(txptrFromEvent->hash()->Get(2), '?');
  ASSERT_EQ(txptrFromEvent->hash()->Get(3), '\0');
  ASSERT_EQ(txptrFromEvent->hash()->size(), 4);

  ASSERT_STREQ(txptrFromEvent->attachment()->mime()->c_str(),
               "=?ISO-2022-JP?B?VG95YW1hX05hbw==?=");

  // validate attachment
  ASSERT_EQ(txptrFromEvent->attachment()->data()->Get(0), 'd');
  ASSERT_EQ(txptrFromEvent->attachment()->data()->Get(1), '\0');
  ASSERT_EQ(txptrFromEvent->attachment()->data()->Get(2), '!');
  ASSERT_EQ(txptrFromEvent->attachment()->data()->size(), 3);
}

TEST(FlatbufferServiceTest, addSignature) {
  flatbuffers::FlatBufferBuilder fbb;

  const auto accountBuf = [&] {
    return flatbuffer_service::CreateAccountBuffer("PublicKey", "Alias\u30e6",
                                                   {"sig1", "sig2", "sig3"}, 1);
  }();

  const auto signatureOffsets = [&] {
    std::vector<uint8_t> sigblob1 = {'a', 'b'};
    std::vector<uint8_t> sigblob2 = {'\0', 'a', '\0', 'b'};
    return std::vector<flatbuffers::Offset<::iroha::Signature>>{
      ::iroha::CreateSignatureDirect(fbb, "TxPubKey1", &sigblob1, 100000),
      ::iroha::CreateSignatureDirect(fbb, "TxPubKey2", &sigblob2, 100001)};
  }();

  const auto _hash = std::vector<uint8_t>{'h', '\0', '?', '\0'};

  const auto attachmentOffset = [&] {
    auto data = std::vector<uint8_t>{'d', '\0', '!'};
    return ::iroha::CreateAttachmentDirect(
      fbb, "=?ISO-2022-JP?B?VG95YW1hX05hbw==?=", &data);
  }();

  const auto txOffset = ::iroha::CreateTransactionDirect(
    fbb, "Creator PubKey", iroha::Command_AccountAdd,
    ::iroha::CreateAccountAddDirect(fbb, &accountBuf).Union(),
    &signatureOffsets, &_hash, attachmentOffset);

  fbb.Finish(txOffset);

  const auto ptr = fbb.ReleaseBufferPointer();
  const auto txptr = flatbuffers::GetRoot<::iroha::Transaction>(ptr.get());

  auto consensusEvent = flatbuffer_service::toConsensusEvent(*txptr);
  ASSERT_TRUE(consensusEvent);

  /*
   * WIP
  flatbuffers::unique_ptr_t uptr;
  consensusEvent.move_value(uptr);

  auto root = flatbuffers::GetRoot<::iroha::ConsensusEvent>(uptr.get());

  auto eventAddedSig1 = flatbuffer_service::addSignature(*root, "NEW PEER PUBLICKEY 1",
                                                         "NEW PEER SIGNATURE 1");
  root = flatbuffers::GetRoot<::iroha::ConsensusEvent>(eventAddedSig1.value().get());

  auto eventAddedSig2 = flatbuffer_service::addSignature(*root, "`1234567890-=",
                                                         "NEW PEER SIGNATURE\'\n\t2");
  root = flatbuffers::GetRoot<::iroha::ConsensusEvent>(eventAddedSig2.value().get());

  auto eventAddedSig3 = flatbuffer_service::addSignature(*root, "NEW\\ PEER PUBLICKEY\'\n\t3",
                                                         "NEW PEER SIGNATURE 3");
  root = flatbuffers::GetRoot<::iroha::ConsensusEvent>(eventAddedSig3.get());

  // validate peerSignatures()
  ASSERT_TRUE(root->peerSignatures()->size() == 3);

  ASSERT_STREQ(root->peerSignatures()->Get(0)->publicKey()->c_str(), "NEW PEER PUBLICKEY 1");
  ASSERT_STREQ(root->peerSignatures()->Get(1)->publicKey()->c_str(), "`1234567890-=");
  ASSERT_STREQ(root->peerSignatures()->Get(2)->publicKey()->c_str(), "NEW\\ PEER PUBLICKEY\'\n\t3");

  //ASSERT_STREQ(root->peerSignatures()->Get(0)->()->c_str(), "NEW PEER PUBLICKEY 1");
  //ASSERT_STREQ(root->peerSignatures()->Get(1)->publicKey()->c_str(), "NEW PEER PUBLICKEY 2");
  //ASSERT_STREQ(root->peerSignatures()->Get(2)->publicKey()->c_str(), "NEW PEER PUBLICKEY 3");

  // validate peer signatures
  //ASSERT_STREQ(root->peerSignatures()->signature());
*/
}
