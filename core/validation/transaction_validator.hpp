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

#ifndef CORE_VALIDATION_TRANSACTIONVALIDATOR_HPP_
#define CORE_VALIDATION_TRANSACTIONVALIDATOR_HPP_

#include <memory>
#include <type_traits>

#include <consensus/consensus_event.hpp>
#include <infra/protobuf/api.grpc.pb.h>

namespace transaction_validator {

  template<typename Signature>
  bool isValid(const Signature &s, const std::string &hash);

  template<typename Signatures>
  bool areValid(const Signatures &s, const std::string &hash);

  template<typename Event>
  bool signaturesAreValid(const Event &tx);

  template<typename Event>
  std::uint32_t countValidSignatures(const Event &event);

};  // namespace transaction_validator

#endif  // CORE_VALIDATION_TRANSACTIONVALIDATOR_HPP_
