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

#ifndef CORE_TRANSACTION_BUILDER_CREATE_OBJECTS_HPP
#define CORE_TRANSACTION_BUILDER_CREATE_OBJECTS_HPP

#include <algorithm>
#include <assert.h>
#include <tuple>

#include <infra/protobuf/api.pb.h>
#include <util/exception.hpp>

namespace txbuilder {

/*
  Primitives
*/

  Api::BaseObject createValueString(std::string val);

  Api::BaseObject createValueInt(int val);

  Api::BaseObject createValueBool(bool val);

  Api::BaseObject createValueDouble(double val);

  Api::Trust createTrust(double value, bool isOk);

/*
  Map
*/

  using Map = std::map<std::string, Api::BaseObject>;

  std::string stringify(::txbuilder::Map m);

/*
  BaseObject
*/

  std::string stringify(Api::BaseObject obj);

/*
  Vector
*/
  template<typename T> using Vector = ::google::protobuf::RepeatedPtrField<T>;

  template<typename T>
  inline std::vector<T> createStandardVector(const ::txbuilder::Vector<T> &protov) {
    return std::vector<T>(protov.begin(), protov.end());
  }

  Api::Domain createDomain(std::string ownerPublicKey, std::string name);

  Api::Account createAccount(std::string publicKey, std::string name,
                             std::vector<std::string> assets);

  Api::Asset createAsset(std::string domain, std::string name,
                         ::txbuilder::Map value, std::string smartContractName);

  Api::SimpleAsset createSimpleAsset(std::string domain, std::string name,
                                     Api::BaseObject value,
                                     std::string smartContractName);

  Api::Peer createPeer(std::string publicKey, std::string address,
                       Api::Trust trust);
} // namespace txbuilder

#endif