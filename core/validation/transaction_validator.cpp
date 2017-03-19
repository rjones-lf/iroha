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

#include <algorithm>
#include <vector>
#include <string>
#include <infra/protobuf/api.pb.h>
#include <crypto/signature.hpp>
#include "transaction_validator.hpp"

namespace transaction_validator {

  using Api::ConsensusEvent;
  using Api::Transaction;
  using signature::verify;

  template<typename Signature>
  bool isValid(const Signature &sig, const std::string &hash) {
    return !verify(sig.signature(), hash, sig.publickey());
  }

  template<typename Signatures>
  bool areValid(const Signatures &s, const std::string &hash) {
    return std::find_if(s.begin(), s.end(),
                        [&hash](const auto &sig) {
                          return isValid(sig, hash);
                        }) == s.end();
  }

  template<typename Signatures>
  std::uint32_t countValid(const Signatures &s, const std::string &hash) {
    return std::count_if(s.begin(), s.end(),
                         [&hash](const auto &sig) {
                           return isValid(sig, hash);
                         });
  }

  template<>
  bool signaturesAreValid<ConsensusEvent>(const ConsensusEvent &ev) {
    auto s = ev.eventsignatures();
    auto tx = ev.transaction();
    return areValid(s, tx.hash());
  }

  template<>
  bool signaturesAreValid<Transaction>(const Transaction &tx) {
    auto s = tx.txsignatures();
    return areValid(s, tx.hash());
  }

  template<>
  std::uint32_t countValidSignatures<ConsensusEvent>(const ConsensusEvent &ev) {
    auto s = ev.eventsignatures();
    auto tx = ev.transaction();
    return countValid(s, tx.hash());
  }

  std::uint32_t countValidSignatures(const Transaction &tx) {
    auto s = tx.txsignatures();
    return countValid(s, tx.hash());
  }
};
