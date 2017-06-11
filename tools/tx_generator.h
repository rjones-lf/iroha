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

#ifndef IROHA_TX_GENERATOR_H
#define IROHA_TX_GENERATOR_H

#include <flatbuffers/flatbuffers.h>
#include <main_generated.h>
#include <vector>
#include <random>
#include <crypto/hash.hpp>
#include <crypto/base64.hpp>
#include <crypto/signature.hpp>

// This is for test and header only. It is better to crate tx_generator.cpp in tools.

// #include <utils/random.hpp>
std::random_device seed_gen;
std::mt19937    rnd_gen_32(seed_gen());
std::mt19937_64 rnd_gen_64(seed_gen());

// return random value of [min, max]
inline int32_t random_value_32(int32_t min, int32_t max) {
  assert(min <= max);
  std::uniform_int_distribution<int32_t> dist(min, max);
  return dist(rnd_gen_32);
}

// return random value of [min, max]
inline uint64_t random_value_64(uint64_t min, uint64_t max) {
  assert(min <= max);
  std::uniform_int_distribution<uint64_t> dist(min, max);
  return dist(rnd_gen_64);
}

namespace generator {

constexpr int MinQuorum = 1;
constexpr int MaxQuorum = 32;
constexpr size_t MaxStringSize = 1e9;

inline char random_alphabet() {
  const std::string buf = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
  const int size = buf.size();
  return buf[random_value_32(0, size - 1)];
}

inline char random_alnum() {
  const std::string buf = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  const int size = buf.size();
  return buf[random_value_32(0, size - 1)];
}

inline std::string random_string(size_t length, std::function<char()> const& char_gen) {
  std::string ret;
  for (size_t i = 0; i < length; i++) {
    ret += char_gen();
  }
  return ret;
}

inline std::string random_alphabets(size_t length) {
  assert(length <= MaxStringSize);
  return random_string(length, random_alphabet);
}

inline std::string random_alnums(size_t length) {
  return random_string(length, random_alnum);
}

inline std::vector<std::uint8_t> random_bytes(size_t length) {
  std::vector<uint8_t> ret;
  for (size_t i = 0; i < length; i++) {
    ret.push_back(random_value_32(0, (1 << 8) - 1));
  }
  return ret;
}

inline std::string random_sha3_256() {
  auto raw_value = random_alphabets(random_value_32(1, 50));
  return hash::sha3_256_hex(raw_value);
}

inline std::string random_base64() {
  auto raw_value = random_alphabets(random_value_32(1, 50));
  std::vector<unsigned char> value;
  for (auto e: raw_value) {
    value.push_back(e);
  }
  return base64::encode(value);
}

inline std::string random_asset_id() {
  return random_alnums(10) + "#"
         + random_alnums(5) + "." + random_alnums(3) + "." + random_alnums(2);
}

inline uint64_t random_time() {
  return random_value_64(0ULL, 1ULL << 63);
}

inline uint32_t random_nonce() {
  return random_value_32(0, 1 << 30);
}

inline std::vector<uint8_t> random_signature(signature::KeyPair const& key_pair) {
  auto message = random_alphabets(50);
  auto res_str = signature::sign(message, key_pair);
  std::vector<uint8_t> res;
  for (auto e: res_str)
    res.push_back((unsigned char)e);
  return res;
}

inline uint8_t random_quorum(int max = MaxQuorum) {
  assert(MinQuorum <= max);
  return random_value_32(MinQuorum, max);
}

inline flatbuffers::Offset<protocol::Signature> random_signature(
  flatbuffers::FlatBufferBuilder &fbb) {
  auto key_pair = signature::generateKeyPair();
  auto pubkey = base64::encode(key_pair.publicKey);
  auto pubkey_v = std::vector<uint8_t>(pubkey.begin(), pubkey.end());

  return protocol::CreateSignature(
    fbb,
    fbb.CreateVector(pubkey_v),
    fbb.CreateVector(random_signature(key_pair))
  );
}

inline std::vector<flatbuffers::Offset<protocol::Signature>> random_signatures(
  flatbuffers::FlatBufferBuilder &fbb, int length = 10) {
  std::vector<flatbuffers::Offset<protocol::Signature>> ret;
  for (int i = 0; i < length; i++) {
    ret.push_back(random_signature(fbb));
  }
  return ret;
}

inline flatbuffers::Offset<protocol::Attachment> random_attachment(
  flatbuffers::FlatBufferBuilder &fbb) {
  return protocol::CreateAttachment(
    fbb, fbb.CreateString(random_alphabets(50)),
    fbb.CreateVector(random_bytes(50)));
}

inline flatbuffers::Offset<protocol::ActionWrapper> random_AccountAddAccount(
  flatbuffers::FlatBufferBuilder &fbb) {
  auto act = protocol::CreateAccountAddAccount(
    fbb, fbb.CreateVector(random_signatures(fbb, 5)), random_quorum(3)
  );
  return protocol::CreateActionWrapper(
    fbb, protocol::Action::AccountAddAccount, act.Union()
  );
}

inline flatbuffers::Offset<protocol::ActionWrapper> random_AssetCreate(
  flatbuffers::FlatBufferBuilder &fbb) {
  auto act = protocol::CreateAssetCreate(
    fbb, fbb.CreateString(random_asset_id())
  );
  return protocol::CreateActionWrapper(
    fbb, protocol::Action::AssetCreate, act.Union()
  );
}

inline std::vector<uint8_t> random_tx(
  flatbuffers::FlatBufferBuilder &fbb,
  std::vector<flatbuffers::Offset<protocol::ActionWrapper>> actions,
  flatbuffers::Offset<protocol::Attachment> attachment) {

  auto tx = protocol::CreateTransaction(
    fbb,
    random_signature(fbb),
    fbb.CreateVector(random_signatures(fbb)),
    random_time(),
    random_nonce(),
    fbb.CreateVector(actions),
    attachment
  );

  fbb.Finish(tx);

  uint8_t *ptr = fbb.GetBufferPointer();
  return {ptr, ptr + fbb.GetSize()};
}

}  // namespace generator

namespace dump {

std::string toString(const flatbuffers::Vector<uint8_t>& blob) {
  std::string ret;
  for (auto e: blob) {
    if (e == '\0') ret += "\\0";
    else ret += e;
  }
  return ret;
}

std::string tab(int indent) {
  if (indent == 0) return "";
  return std::string(indent, ' ');
}

std::string toString(protocol::Signature const& signature,
                     std::string const& name, int indent = 0) {
  std::string res;
  res += tab(indent) + name + "[\n";
  res += tab(indent) + "  pubkey:" + toString(*signature.pubkey()) + ",\n";
  res += tab(indent) + "  sig:"    + toString(*signature.sig()) + "\n";
  res += tab(indent) + "]\n";
  return res;
}

std::string toString(flatbuffers::Vector<
  flatbuffers::Offset<protocol::Signature>> const& sigs,
  std::string const& name, int indent = 0
) {
  std::string res;
  res += tab(indent) + name + "[\n";
  for (size_t i = 0; i < sigs.size(); ++i) {
    res += toString(*sigs.Get(i), std::to_string(i), indent + 2);
  }
  res += tab(indent) + "]\n";
  return res;
}

std::string toString(const protocol::Transaction& tx) {
  std::string res = "";

  if (tx.creator() != nullptr) {
    res += toString(*tx.creator(), "creator");
  }

  if (tx.sigs() != nullptr) {
    res += "sigs:[\n";
    for (const auto& s : *tx.sigs()) {
      if (s->pubkey() != nullptr && s->sig() != nullptr) {
        res += "  [\n";
        res += "    pubkey:" + toString(*s->pubkey()) + ",\n";
        res += "    sig:"    + toString(*s->sig()) + "\n";
        res += "  ]\n";
      } else {
        res += "[broken]\n";
      }
    }
    res += "]\n";
  }

  if (tx.attachment() != nullptr) {
    assert(tx.attachment()->mime() != nullptr);
    assert(tx.attachment()->data() != nullptr);

    res += "attachment:[\n";
    res += "  mime:" + tx.attachment()->mime()->str() + ",\n";
    res += "  data:" + toString(*tx.attachment()->data()) + "\n";
    res += "]\n";
  }

  std::map<protocol::Action, std::function<std::string(const void*)>>
    action_to_strings;

  action_to_strings[protocol::Action::AccountAddAccount] =
    [&](const void* action) -> std::string {
      const protocol::AccountAddAccount* act =
        static_cast<const protocol::AccountAddAccount*>(action);

      std::string res = "AccountAddAccount[\n";
      res += toString(*act->signatories(), "signatories", 2);
      res += "  quorum:" + std::to_string(act->quorum()) + "\n";
      res += "]\n";
      return res;
    };

  action_to_strings[protocol::Action::AssetCreate] =
    [&](const void* action) -> std::string {
      const protocol::AssetCreate* act =
        static_cast<const protocol::AssetCreate*>(action);

      std::string res = "AssetCreate[\n";
      res += "  asset_id: " + act->asset_id()->str() + "\n";
      res += "]\n";
      return res;
    };

  for (auto const& act: *tx.actions()) {
    if (action_to_strings.find(act->action_type()) == action_to_strings.end()) {
      throw std::runtime_error("The action's dump is not implemented yet.");
    }
    res += action_to_strings[act->action_type()](act->action());
  }

  return res;
}

}  // namespace dump

#endif //IROHA_TX_GENERATOR_H
