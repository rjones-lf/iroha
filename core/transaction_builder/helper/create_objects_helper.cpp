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

#include <infra/protobuf/api.pb.h>
#include <util/exception.hpp>

#include "create_objects_helper.hpp"

namespace txbuilder {

/*
  Primitives
*/

  Api::BaseObject createValueString(std::string val) {
    Api::BaseObject ret;
    ret.set_valuestring(std::move(val));
    return ret;
  }

  Api::BaseObject createValueInt(int val) {
    Api::BaseObject ret;
    ret.set_valueint(static_cast<::google::protobuf::int64>(val));
    return ret;
  }

  Api::BaseObject createValueBool(bool val) {
    Api::BaseObject ret;
    ret.set_valueboolean(val);
    return ret;
  }

  Api::BaseObject createValueDouble(double val) {
    Api::BaseObject ret;
    ret.set_valuedouble(val);
    return ret;
  }

  Api::Trust createTrust(double value, bool isOk) {
    Api::Trust ret;
    ret.set_value(value);
    ret.set_isok(isOk);
    return ret;
  }

/*
  Map
*/

  using Map = std::map<std::string, Api::BaseObject>;

  std::string stringify(::txbuilder::Map m) {
    std::string ret = "{";
    for (auto &&e : m) {
      ret += "(" + e.first + ", " + txbuilder::stringify(e.second) + "), ";
    }
    ret += "}";
    return ret;
  }

/*
  BaseObject
*/

  std::string stringify(Api::BaseObject obj) {
    switch (obj.value_case()) {
      case Api::BaseObject::ValueCase::kValueString:
        return obj.valuestring();
      case Api::BaseObject::ValueCase::kValueInt:
        return std::to_string(obj.valueint());
      case Api::BaseObject::ValueCase::kValueBoolean:
        return obj.valueboolean() ? std::string("true") : "false";
      case Api::BaseObject::ValueCase::kValueDouble:
        return std::to_string(obj.valuedouble());
      default:
        throw "invalid type exception";
    }
  }

/*
  Vector
*/
  template<typename T> using Vector = ::google::protobuf::RepeatedPtrField<T>;

/*
  Assets
*/
  Api::Domain createDomain(std::string ownerPublicKey, std::string name) {
    Api::Domain ret;
    ret.set_ownerpublickey(std::move(ownerPublicKey));
    ret.set_name(std::move(name));
    return ret;
  }

  Api::Account createAccount(std::string publicKey, std::string name,
                             std::vector<std::string> assets) {
    Api::Account ret;
    ret.set_publickey(std::move(publicKey));
    ret.set_name(std::move(name));
    *ret.mutable_assets() = ::google::protobuf::RepeatedPtrField<std::string>(
        assets.begin(), assets.end());
    return ret;
  }

  Api::Asset createAsset(
      std::string domain,
      std::string name,
      ::txbuilder::Map value, std::string smartContractName) {
    Api::Asset ret;
    ret.set_domain(std::move(domain));
    ret.set_name(std::move(name));
    *ret.mutable_value() = ::google::protobuf::Map<std::string, Api::BaseObject>(
        value.begin(), value.end());
    return ret;
  }

  Api::SimpleAsset createSimpleAsset(std::string domain, std::string name,
                                     Api::BaseObject value,
                                     std::string smartContractName) {
    Api::SimpleAsset ret;
    ret.set_domain(std::move(domain));
    ret.set_name(std::move(name));
    *ret.mutable_value() = std::move(value);
    return ret;
  }

  Api::Peer createPeer(std::string publicKey, std::string address,
                       Api::Trust trust) {
    Api::Peer ret;
    ret.set_publickey(std::move(publicKey));
    ret.set_address(std::move(address));
    *ret.mutable_trust() = std::move(trust);
    return ret;
  }
}
