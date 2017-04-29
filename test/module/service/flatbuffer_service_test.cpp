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
#include <gtest/gtest.h>
#include <service/flatbuffer_service.h>
#include <membership_service/peer_service.hpp>
#include <utils/datetime.hpp>
#include <main_generated.h>

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

TEST(FlatbufferServiceTest, toConsensusEvent_AccountAdd) {
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
    fbb, "Creator PubKey", iroha::Command::AccountAdd,
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
  ASSERT_EQ(root->code(), ::iroha::Code::UNDECIDED);

  // validate transactions()
  const auto txptrFromEvent =
    root->transactions()->Get(0)->tx_nested_root();  // ToDo: toConsensusEvent() receives 1 tx.

  ASSERT_STREQ(txptrFromEvent->creatorPubKey()->c_str(), "Creator PubKey");
  ASSERT_EQ(txptrFromEvent->command_type(), ::iroha::Command::AccountAdd);

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

TEST(FlatbufferServiceTest, addSignature_AccountAdd) {
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
    fbb, "Creator PubKey", iroha::Command::AccountAdd,
    ::iroha::CreateAccountAddDirect(fbb, &accountBuf).Union(),
    &signatureOffsets, &_hash, attachmentOffset);

  fbb.Finish(txOffset);

  const auto ptr = fbb.ReleaseBufferPointer();
  const auto txptr = flatbuffers::GetRoot<::iroha::Transaction>(ptr.get());

  auto consensusEvent = flatbuffer_service::toConsensusEvent(*txptr);
  ASSERT_TRUE(consensusEvent);

  flatbuffers::unique_ptr_t uptr;
  consensusEvent.move_value(uptr);

  auto root = flatbuffers::GetRoot<::iroha::ConsensusEvent>(uptr.get());

  const auto timeSig1Begin = datetime::unixtime();
  auto eventAddedSig1 = flatbuffer_service::addSignature(*root, "NEW PEER PUBLICKEY 1",
                                                         "NEW PEER SIGNATURE 1");
  const auto timeSig1End = datetime::unixtime();
  root = flatbuffers::GetRoot<::iroha::ConsensusEvent>(eventAddedSig1.value().get());
  ASSERT_LE(timeSig1Begin, root->peerSignatures()->Get(0)->timestamp());
  ASSERT_LE(root->peerSignatures()->Get(0)->timestamp(), timeSig1End);
  ASSERT_EQ(root->code(), ::iroha::Code::UNDECIDED);

  const auto timeSig2Begin = datetime::unixtime();
  auto eventAddedSig2 = flatbuffer_service::addSignature(*root, "`1234567890-=",
                                                         "NEW PEER SIGNATURE\'\n\t2");
  const auto timeSig2End = datetime::unixtime();
  root = flatbuffers::GetRoot<::iroha::ConsensusEvent>(eventAddedSig2.value().get());
  ASSERT_LE(timeSig2Begin, root->peerSignatures()->Get(1)->timestamp());
  ASSERT_LE(root->peerSignatures()->Get(1)->timestamp(), timeSig2End);
  ASSERT_EQ(root->code(), ::iroha::Code::UNDECIDED);

  const auto timeSig3Begin = datetime::unixtime();
  auto eventAddedSig3 = flatbuffer_service::addSignature(*root, "NEW\\ PEER PUBLICKEY\'\n\t3\u30e6\u30e6",
                                                         "NEW PEER SIGNATURE 3\u30e6\u30e6");
  const auto timeSig3End = datetime::unixtime();
  root = flatbuffers::GetRoot<::iroha::ConsensusEvent>(eventAddedSig3.value().get());
  ASSERT_LE(timeSig3Begin, root->peerSignatures()->Get(2)->timestamp());
  ASSERT_LE(root->peerSignatures()->Get(2)->timestamp(), timeSig3End);
  ASSERT_EQ(root->code(), ::iroha::Code::UNDECIDED);

  // validate peerSignatures()
  ASSERT_TRUE(root->peerSignatures()->size() == 3);

  ASSERT_STREQ(root->peerSignatures()->Get(0)->publicKey()->c_str(), "NEW PEER PUBLICKEY 1");
  ASSERT_STREQ(root->peerSignatures()->Get(1)->publicKey()->c_str(), "`1234567890-=");
  ASSERT_STREQ(root->peerSignatures()->Get(2)->publicKey()->c_str(), "NEW\\ PEER PUBLICKEY\'\n\t3\u30e6\u30e6");

  // ToDo: GetAsString(uoffset_t) is better, but I don't know how to do.
  auto sigs = root->peerSignatures();
  std::string sig1(sigs->Get(0)->signature()->begin(), sigs->Get(0)->signature()->end());
  std::string sig2(sigs->Get(1)->signature()->begin(), sigs->Get(1)->signature()->end());
  std::string sig3(sigs->Get(2)->signature()->begin(), sigs->Get(2)->signature()->end());
  ASSERT_STREQ(sig1.c_str(), "NEW PEER SIGNATURE 1");
  ASSERT_STREQ(sig2.c_str(), "NEW PEER SIGNATURE\'\n\t2");
  ASSERT_STREQ(sig3.c_str(), "NEW PEER SIGNATURE 3\u30e6\u30e6");

  // validate transactions()
  const auto txptrFromEvent =
    root->transactions()->Get(0)->tx_nested_root();  // ToDo: toConsensusEvent() receives 1 tx.

  ASSERT_STREQ(txptrFromEvent->creatorPubKey()->c_str(), "Creator PubKey");
  ASSERT_EQ(txptrFromEvent->command_type(), ::iroha::Command::AccountAdd);

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

TEST(FlatbufferServiceTest, makeCommit_AccountAdd) {
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
    fbb, "Creator PubKey", iroha::Command::AccountAdd,
    ::iroha::CreateAccountAddDirect(fbb, &accountBuf).Union(),
    &signatureOffsets, &_hash, attachmentOffset);

  fbb.Finish(txOffset);

  const auto ptr = fbb.ReleaseBufferPointer();
  const auto txptr = flatbuffers::GetRoot<::iroha::Transaction>(ptr.get());

  auto consensusEvent = flatbuffer_service::toConsensusEvent(*txptr);
  ASSERT_TRUE(consensusEvent);

  flatbuffers::unique_ptr_t uptr;
  consensusEvent.move_value(uptr);

  auto root = flatbuffers::GetRoot<::iroha::ConsensusEvent>(uptr.get());

  auto addedSigEvent = flatbuffer_service::addSignature(*root, "NEW PEER PUBLICKEY 1",
                                                        "NEW PEER SIGNATURE 1");
  root = flatbuffers::GetRoot<::iroha::ConsensusEvent>(addedSigEvent.value().get());

  auto committedEvent = flatbuffer_service::makeCommit(*root);
  ASSERT_TRUE(committedEvent);

  committedEvent.move_value(uptr);

  root = flatbuffers::GetRoot<::iroha::ConsensusEvent>(uptr.get());

  // validate peerSignatures()
  ASSERT_TRUE(root->peerSignatures()->size() == 1);
  ASSERT_EQ(root->code(), ::iroha::Code::COMMIT);

  // validate transactions()
  const auto txptrFromEvent =
    root->transactions()->Get(0)->tx_nested_root();  // ToDo: toConsensusEvent() receives 1 tx.

  ASSERT_STREQ(txptrFromEvent->creatorPubKey()->c_str(), "Creator PubKey");
  ASSERT_EQ(txptrFromEvent->command_type(), ::iroha::Command::AccountAdd);

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

TEST(FlatbufferServiceTest, copyConsensusEvent) {
  flatbuffers::FlatBufferBuilder xbb;
  std::vector<flatbuffers::Offset<::iroha::Signature>> peerSignatures;
  std::vector<uint8_t> signature = {'a','b','c'};
  peerSignatures.push_back(::iroha::CreateSignatureDirect(xbb, "PUBKEY1", &signature, 100000));
  peerSignatures.push_back(::iroha::CreateSignatureDirect(xbb, "PUBKEY2", &signature, 200000));

  std::vector<std::string> signatories{"123","-=[p"};
  auto account = flatbuffer_service::CreateAccountBuffer("accpubkey", "alias", signatories, 1);
  auto command = ::iroha::CreateAccountAddDirect(xbb, &account);
  std::vector<flatbuffers::Offset<::iroha::Signature>> tx_signatures;
  std::vector<uint8_t> txsig = {'a','b','c'};
  tx_signatures.push_back(::iroha::CreateSignatureDirect(xbb, "txsig pubkey 1", &txsig, 99999));
  std::vector<uint8_t> hash = {'h','s'};
  std::vector<uint8_t> data = {'d','t'};
  auto attachment = ::iroha::CreateAttachmentDirect(xbb, "mime", &data);
  auto txoffset = ::iroha::CreateTransactionDirect(xbb, "creator", ::iroha::Command::AccountAdd, command.Union(), &tx_signatures, &hash, attachment);
  xbb.Finish(txoffset);
  std::vector<uint8_t> txbuf(xbb.GetBufferPointer(), xbb.GetBufferPointer() + xbb.GetSize());

  flatbuffers::FlatBufferBuilder ebb;
  auto txw = ::iroha::CreateTransactionWrapperDirect(ebb, &txbuf);
  std::vector<flatbuffers::Offset<::iroha::TransactionWrapper>> txwrappers;
  txwrappers.push_back(txw);
  auto eventofs = ::iroha::CreateConsensusEventDirect(ebb, &peerSignatures, &txwrappers, ::iroha::Code::UNDECIDED);
  ebb.Finish(eventofs);

  auto eflatbuf = ebb.ReleaseBufferPointer();
  auto eventptr = flatbuffers::GetRoot<::iroha::ConsensusEvent>(eflatbuf.get());

  flatbuffers::FlatBufferBuilder copyebb;
  auto copyeventofs = flatbuffer_service::copyConsensusEvent(copyebb, *eventptr);
  ASSERT_TRUE(copyeventofs);
  copyebb.Finish(copyeventofs.value());

  auto copyeventbuf = copyebb.ReleaseBufferPointer();
  auto copyeventptr = flatbuffers::GetRoot<::iroha::ConsensusEvent>(copyeventbuf.get());

  ASSERT_EQ(copyeventptr->code(), ::iroha::Code::UNDECIDED);
  auto revPeerSigs = copyeventptr->peerSignatures();
  ASSERT_STREQ(revPeerSigs->Get(0)->publicKey()->c_str(), "PUBKEY1");
  ASSERT_STREQ(revPeerSigs->Get(1)->publicKey()->c_str(), "PUBKEY2");
  std::vector<uint8_t> sig1(revPeerSigs->Get(0)->signature()->begin(), revPeerSigs->Get(0)->signature()->end());
  std::vector<uint8_t> sig2(revPeerSigs->Get(1)->signature()->begin(), revPeerSigs->Get(1)->signature()->end());
  std::vector<uint8_t> abc = {'a','b','c'};
  ASSERT_EQ(sig1, abc);
  ASSERT_EQ(sig2, abc);
  ASSERT_EQ(revPeerSigs->Get(0)->timestamp(), 100000);
  ASSERT_EQ(revPeerSigs->Get(1)->timestamp(), 200000);

  auto txnested = copyeventptr->transactions()->Get(0)->tx_nested_root();
  ASSERT_STREQ(txnested->creatorPubKey()->c_str(), "creator");
  ASSERT_EQ(txnested->command_type(), ::iroha::Command::AccountAdd);
  auto txnestedsig = txnested->signatures()->Get(0);
  ASSERT_STREQ(txnestedsig->publicKey()->c_str(), "txsig pubkey 1");
  ASSERT_EQ(txnestedsig->signature()->size(), 3);
  ASSERT_EQ(txnestedsig->signature()->Get(0), 'a');
  ASSERT_EQ(txnestedsig->signature()->Get(1), 'b');
  ASSERT_EQ(txnestedsig->signature()->Get(2), 'c');
  ASSERT_EQ(txnestedsig->timestamp(), 99999);
  ASSERT_EQ(txnested->hash()->Get(0), 'h');
  ASSERT_EQ(txnested->hash()->Get(1), 's');
  ASSERT_STREQ(txnested->attachment()->mime()->c_str(), "mime");
  ASSERT_EQ(txnested->attachment()->data()->Get(0), 'd');
  ASSERT_EQ(txnested->attachment()->data()->Get(1), 't');
  auto accnested = txnested->command_as_AccountAdd()->account_nested_root();
  ASSERT_STREQ(accnested->pubKey()->c_str(), "accpubkey");
  ASSERT_STREQ(accnested->alias()->c_str(), "alias");
  ASSERT_STREQ(accnested->signatories()->Get(0)->c_str(), "123");
  ASSERT_STREQ(accnested->signatories()->Get(1)->c_str(), "-=[p");
  ASSERT_EQ(accnested->useKeys(), 1);
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
  auto setTrust = flatbuffer_service::peer::CreateChangeTrust("pubKey", trust);
  fbb.Finish(setTrust);
  auto setTrustPtr = flatbuffers::GetRoot<iroha::PeerChangeTrust>(fbb.GetBufferPointer());
  //ASSERT_EQ(setTrustPtr->command_type(), ::iroha::Command::PeerChangeTrust);
  ASSERT_STREQ(setTrustPtr->peerPubKey()->c_str(), "pubKey");
  ASSERT_EQ(setTrustPtr->delta(), 3.14159265);
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
