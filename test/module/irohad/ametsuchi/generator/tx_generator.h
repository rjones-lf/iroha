/**
 * Copyright Soramitsu Co., Ltd. 2017 All Rights Reserved.
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

#ifndef AMETSUCHI_TX_GENERATOR_H
#define AMETSUCHI_TX_GENERATOR_H

#include <algorithm>
#include <functional>
#include <iostream>
#include <string>
#include <vector>
#include <transaction_generated.h>
#include <account_generated.h>

namespace generator {

/**
 * Default ledger and domain for currency
 */
std::string LEDGER_ = "ledger_default";
std::string DOMAIN_ = "domain_default";
std::string INIT_AMOUNT = "100";

    /**
 * Standart lengths of cryptographic primitives
 */
size_t HASH_SIZE_BLOB_ = 32;
size_t PUB_KEY_LENGTH_STR_ = 44;
size_t SIGNATURE_LENGTH_BLOB_ = 44;

/**
 * Current state of random generators.
 */
unsigned int SEED_ = 1337;

const char ALPHABET[] =
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

std::string random_base64_string(size_t length) {
  const char alph[] =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  std::string s;
  std::generate_n(std::back_inserter(s), length, [&alph]() {
    auto i = generator::random_number(0, sizeof(alph) - 1);
    return (char)alph[i];
  });

  if (s.size() % 4 == 0) {
    s.pop_back();
    s.push_back('=');
  }
  return s;
}


std::string random_public_key() {
  return random_base64_string(PUB_KEY_LENGTH_STR_);
}


std::vector<uint8_t> random_blob(size_t length) {
  std::vector<uint8_t> v(length);
  std::generate_n(v.begin(), length, std::bind(random_number, 0, 256));
  return v;
}


std::string random_ip() {
  std::string s;
  s += std::to_string(random_number(0, 256));
  s += '.';
  s += std::to_string(random_number(0, 256));
  s += '.';
  s += std::to_string(random_number(0, 256));
  s += '.';
  s += std::to_string(random_number(0, 256));
  return s;
}


std::vector<uint8_t> random_account(std::string pubkey = random_public_key(),
                                    std::string alias = random_string(10),
                                    uint16_t signatories = (uint16_t)
                                        random_number(1, 10)) {
  flatbuffers::FlatBufferBuilder fbb(2048);

  std::vector<std::string> sign(signatories);
  std::generate_n(sign.begin(), signatories, random_public_key);

  auto account = iroha::CreateAccount(
      fbb, fbb.CreateString(pubkey), fbb.CreateString(""), fbb.CreateString(alias),
      fbb.CreateVectorOfStrings(sign), signatories);

  fbb.Finish(account);

  uint8_t* ptr = fbb.GetBufferPointer();
  return {ptr, ptr + fbb.GetSize()};
}


std::vector<uint8_t> random_peer(std::string ledger_name = random_string(10),
                                 std::string pubkey = random_public_key(),
                                 std::string ip = random_ip(),
                                 double trust = random_number(0, 10)) {
  flatbuffers::FlatBufferBuilder fbb(2048);


  auto peer = iroha::CreatePeer(fbb, fbb.CreateString(ledger_name),
                                fbb.CreateString(pubkey),
                                fbb.CreateString(ip), trust);

  fbb.Finish(peer);

  uint8_t* ptr = fbb.GetBufferPointer();
  return {ptr, ptr + fbb.GetSize()};
}

std::vector<uint8_t> random_currency(
    uint64_t amount = (uint64_t)random_number(1, 50),
    uint8_t precision = (uint8_t)random_number(0, 3),
    std::string currency_name = random_string(6),
    std::string domain_name = DOMAIN_, std::string ledger_name = LEDGER_,
    std::string description = random_string((size_t)random_number(5, 100))) {
  flatbuffers::FlatBufferBuilder fbb(2048);

  auto currency = iroha::CreateCurrency(
      fbb, fbb.CreateString(currency_name), fbb.CreateString(domain_name),
      fbb.CreateString(ledger_name), fbb.CreateString(description), fbb.CreateString(std::to_string(amount)),
      precision);

  fbb.Finish(currency);

  uint8_t* ptr = fbb.GetBufferPointer();
  return {ptr, ptr + fbb.GetSize()};
}

std::vector<uint8_t> random_asset_wrapper_currency(
    uint64_t amount = (uint64_t)random_number(1, 50),
    uint8_t precision = (uint8_t)random_number(0, 3),
    std::string currency_name = random_string(6),
    std::string domain_name = DOMAIN_, std::string ledger_name = LEDGER_,
    std::string description = random_string(0)) {
  flatbuffers::FlatBufferBuilder fbb(2048);

  // May be erased commen :
  /*
   * Even though that two asset has same currency_name and domain_name and
   * ledgername,
   * not necessary two asset same size.
   */
  // printf("in rando gen\n");
  // printf("%d %d %s %s %s
  // %s\n",amount,precision,currency_name.c_str(),domain_name.c_str(),ledger_name.c_str(),description.c_str());
  auto asset = iroha::CreateAsset(
      fbb, iroha::AnyAsset::Currency,
      iroha::CreateCurrency(fbb, fbb.CreateString(currency_name),
                            fbb.CreateString(domain_name),
                            fbb.CreateString(ledger_name),
                            fbb.CreateString(description), fbb.CreateString(std::to_string(amount)), precision)
          .Union());

  fbb.Finish(asset);

  uint8_t* ptr = fbb.GetBufferPointer();
  // printf("random_asset_wrapper_currency size: %d\n",fbb.GetSize());
  // fflush(stdout);
  return {ptr, ptr + fbb.GetSize()};
}


flatbuffers::Offset<iroha::Signature> random_signature(
    flatbuffers::FlatBufferBuilder& fbb,
    const std::string pubk = random_public_key(),
    const std::vector<uint8_t> signature = random_blob(SIGNATURE_LENGTH_BLOB_),
    uint64_t timestamp = (uint64_t)random_number(0, 1 << 30)) {
  return iroha::CreateSignature(fbb, fbb.CreateString(pubk),
                                fbb.CreateVector(signature), timestamp);
}


flatbuffers::Offset<iroha::AccountAdd> random_AccountAdd(
    flatbuffers::FlatBufferBuilder& fbb,
    const std::vector<uint8_t> account = random_account()) {
  return iroha::CreateAccountAdd(fbb, fbb.CreateVector(account));
}


flatbuffers::Offset<iroha::AccountRemove> random_AccountRemove(
    flatbuffers::FlatBufferBuilder& fbb,
    const std::string accPubKey = random_public_key()) {
  return iroha::CreateAccountRemove(fbb, fbb.CreateString(accPubKey));
}


flatbuffers::Offset<iroha::Add> random_Add(
    flatbuffers::FlatBufferBuilder& fbb,
    std::string accPubKey = random_public_key(),
    std::vector<uint8_t> asset = random_asset_wrapper_currency()) {
  return iroha::CreateAdd(fbb, fbb.CreateString(accPubKey),
                               fbb.CreateVector(asset));
}


flatbuffers::Offset<iroha::Subtract> random_Subtract(
    flatbuffers::FlatBufferBuilder& fbb,
    std::string accPubKey = random_public_key(),
    std::vector<uint8_t> asset = random_asset_wrapper_currency()) {
  return iroha::CreateSubtract(fbb, fbb.CreateString(accPubKey),
                                  fbb.CreateVector(asset));
}


flatbuffers::Offset<iroha::AssetCreate> random_AssetCreate(
    flatbuffers::FlatBufferBuilder& fbb,
    std::string currency_name = random_string(10),
    std::string domain_name = DOMAIN_, std::string ledger_name = LEDGER_,
    std::string init_amount = INIT_AMOUNT) {
  return iroha::CreateAssetCreate(fbb, fbb.CreateString(currency_name),
                                  fbb.CreateString(domain_name),
                                  fbb.CreateString(ledger_name),
                                  fbb.CreateString(init_amount));
}


flatbuffers::Offset<iroha::Transfer> random_Transfer(
    flatbuffers::FlatBufferBuilder& fbb,
    std::vector<uint8_t> asset = random_asset_wrapper_currency(),
    std::string sender = random_public_key(),
    std::string receiver = random_public_key()) {
  return iroha::CreateTransfer(fbb, fbb.CreateVector(asset),
                                    fbb.CreateString(sender),
                                    fbb.CreateString(receiver));
}


flatbuffers::Offset<iroha::PeerAdd> random_PeerAdd(
    flatbuffers::FlatBufferBuilder& fbb,
    std::vector<uint8_t> peer = random_peer()) {
  return iroha::CreatePeerAdd(fbb, fbb.CreateVector(peer));
}


flatbuffers::Offset<iroha::PeerRemove> random_PeerRemove(
    flatbuffers::FlatBufferBuilder& fbb,
    std::string pubkey = random_string(10) ) {
  return iroha::CreatePeerRemove(fbb, fbb.CreateString(pubkey));
}

/**
 * Returns deserialized transaction (root flatbuffer)
 * @param fbb - a reference to flatbuffer builder.
 * @param cmd_type - one of iroha::Command
 * @param command - random_*(fbb).Union() where * is the same type as \p
 * cmd_type. Example: cmd_type=iroha::Command::PeerRemove,
 * command=random_PeerRemove(fbb).Union()
 * @param signatures - number of signatures
 * @param creator - random public key of a creator
 * @param hash - random hash of a transaction
 * @return ready to be transmitted/parsed root Transaction flatbuffer
 */
std::vector<uint8_t> random_transaction(
    flatbuffers::FlatBufferBuilder& fbb, iroha::Command cmd_type,
    flatbuffers::Offset<void> command, const size_t signatures = 5,
    std::string creator = random_public_key(),
    std::vector<uint8_t> hash = random_blob(HASH_SIZE_BLOB_)) {
  std::vector<flatbuffers::Offset<iroha::Signature>> sigs(signatures);
  std::generate_n(sigs.begin(), signatures,
                  [&fbb]() { return random_signature(fbb); });

  auto tx = iroha::CreateTransaction(fbb, fbb.CreateString(creator), cmd_type,
                                     command, fbb.CreateVector(sigs),
                                     fbb.CreateVector(hash));

  fbb.Finish(tx);

  uint8_t* ptr = fbb.GetBufferPointer();
  return {ptr, ptr + fbb.GetSize()};
}

}  // namespace generator

#endif  // AMETSUCHI_TX_GENERATOR_H
