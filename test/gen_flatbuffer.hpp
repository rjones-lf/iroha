/**
 * Copyright 2016 Soramitsu Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include <flatbuffers/flatbuffers.h>
#include <main_generated.h>

template <typename T>
using Offset = flatbuffers::Offset<T>;

inline std::vector<uint8_t> toVector(const std::string& s) {
  std::vector<uint8_t> ret;
  for (auto& e: s) { ret.push_back(e); }
  return ret;
}

namespace gen_flatbuffer {

inline std::vector <uint8_t> CreateTx() {
  flatbuffers::FlatBufferBuilder fbb;
  auto creator_pubkey = toVector("hoge");
  auto signature = toVector("foo");
  auto creator = protocol::CreateSignatureDirect(
    fbb, &creator_pubkey, &signature
  );

  auto abc = toVector("abc");
  auto def = toVector("def");
  auto sigs = fbb.CreateVector < Offset < protocol::Signature >> (
    std::vector < Offset < protocol::Signature >> {
      protocol::CreateSignatureDirect(
        fbb, &abc, &def
      )}
  );

  auto actions = fbb.CreateVector < flatbuffers::Offset < protocol::ActionWrapper >> ({
    protocol::CreateActionWrapper(
      fbb, protocol::Action::AccountAddAccount,
      protocol::CreateAccountAddAccount(
        fbb, fbb.CreateString("user"), sigs, uint8_t(16)
      ).Union()
    )
  });

  auto txo = protocol::CreateTransaction(
    fbb, creator, sigs, 1234567890, 987656789, actions
  );

  fbb.Finish(txo);

  auto buf = fbb.GetBufferPointer();
  return {buf, buf + fbb.GetSize()};
}

} // namespace gen_flatbuffer