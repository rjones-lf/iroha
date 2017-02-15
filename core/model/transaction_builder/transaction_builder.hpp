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

#ifndef TRANSACTION_BUILDER_HPP
#define TRANSACTION_BUILDER_HPP

#include "../objects/domain.hpp"
#include "../objects/account.hpp"
#include "../objects/asset.hpp"
#include "../objects/message.hpp"
#include "../objects/simple_asset.hpp"
#include "../objects/peer.hpp"

namespace transaction {

template<class BuildObjectT>
class TransactionBuilder {
  TransactionBuilder() = default;
  TransactionBuilder(const TransactionBuilder&) = default;
  TransactionBuilder(TransactionBuilder&&) = default;
};

/*****************************************************************************
  object::Domain
 *****************************************************************************/
template<>
class TransactionBuilder<object::Domain> {
public:

  TransactionBuilder() = default;
  TransactionBuilder(const TransactionBuilder&) = default;
  TransactionBuilder(TransactionBuilder&&) = default;

  TransactionBuilder& setOwnerPublicKey(std::vector<std::string> ownerPublicKey) {
    _ownerPublicKey = std::move(ownerPublicKey);
    return *this;
  }

  TransactionBuilder& setName(std::string name) {
    _name = std::move(name);
    return *this;
  }

  object::Domain build() {
    const auto unsetMembers = enumerateUnsetMembers();
    if (not unsetMembers.empty()) {
      throw exception::transaction::UnsetBuildArgmentsException("object::Domain", unsetMembers);
    }

    return object::Domain(_ownerPublicKey, _name);
  }

private:

  std::string enumerateUnsetMembers() {    
    std::string ret;
    if (_ownerPublicKey.empty()) ret += " ownerPublicKey";
    if (_name.empty())           ret += " name";
    return ret;
  }

  std::vector<std::string> _ownerPublicKey;
  std::string _name;
};


/*****************************************************************************
  object::Account
 *****************************************************************************/
template<>
class TransactionBuilder<object::Account> {
public:

  TransactionBuilder() = default;
  TransactionBuilder(const TransactionBuilder&) = default;
  TransactionBuilder(TransactionBuilder&&) = default;

  TransactionBuilder& setOwnerPublicKey(std::string ownerPublicKey) {
    _ownerPublicKey = std::move(ownerPublicKey);
    return *this;
  }

  TransactionBuilder& setName(std::string name) {
    _name = std::move(name);
    return *this;
  }

  TransactionBuilder& setAssets(std::vector<object::Domain> assets) {
    _assets = std::move(assets);
    return *this;
  }

  object::Account build() {
    const auto unsetMembers = enumerateUnsetMembers();
    if (not unsetMembers.empty()) {
      throw exception::transaction::UnsetBuildArgmentsException("object::Account", unsetMembers);
    }
    return object::Account(_ownerPublicKey, _name, _assets);
  }

private:

  std::string enumerateUnsetMembers() {
    std::string ret;
    if (_ownerPublicKey.empty())  ret += " ownerPublicKey";
    if (_name.empty())            ret += " name";
    if (_assets.empty())          ret += " assets";
    return ret;
  }

  std::string _ownerPublicKey;
  std::string _name;
  std::vector<object::Domain> _assets;
};

/*****************************************************************************
  object::Asset
 *****************************************************************************/
template<>
class TransactionBuilder<object::Asset> {
public:

  TransactionBuilder() = default;
  TransactionBuilder(const TransactionBuilder&) = default;
  TransactionBuilder(TransactionBuilder&&) = default;

  TransactionBuilder& setDomain(std::string domain) {
    _domain = std::move(domain);
    return *this;
  }

  TransactionBuilder& setName(std::string name) {
    _name = std::move(name);
    return *this;
  }

  TransactionBuilder& setValue(object::AssetValueT value) {
    _value = std::move(value);
    return *this;
  }

  object::Asset build() {

    const auto unsetMembers = enumerateUnsetMembers();
    if (not unsetMembers.empty()) {
      throw exception::transaction::UnsetBuildArgmentsException("object::Asset", unsetMembers);
    }

    return object::Asset(_domain, _name, _value);
  }

private:

  std::string enumerateUnsetMembers() {
    std::string ret;
    if (_domain.empty())  ret += " domain";
    if (_name.empty())    ret += " name";
    if (_value.empty())   ret += " value";
    return ret;
  }

  std::string _domain;
  std::string _name;
  object::AssetValueT _value;
};

/*****************************************************************************
  object::Message
 *****************************************************************************/
template<>
class TransactionBuilder<object::Message> {
public:

  TransactionBuilder() = default;
  TransactionBuilder(const TransactionBuilder&) = default;
  TransactionBuilder(TransactionBuilder&&) = default;

  TransactionBuilder& setMessage(std::string text) {
    _text = std::move(text);
    return *this;
  }

  object::Message build() {

    const auto unsetMembers = enumerateUnsetMembers();
    if (not unsetMembers.empty()) {
      throw exception::transaction::UnsetBuildArgmentsException("object::Message", unsetMembers);
    }

    return object::Message(_text);
  }

private:

  std::string enumerateUnsetMembers() {
    std::string ret;
    if (_text.empty()) ret += " text";
    return ret;
  }

  std::string _text;
};

/*****************************************************************************
  object::SimpleAsset
 *****************************************************************************/
template<>
class TransactionBuilder<object::SimpleAsset> {
public:

  TransactionBuilder() = default;
  TransactionBuilder(const TransactionBuilder&) = default;
  TransactionBuilder(TransactionBuilder&&) = default;

  TransactionBuilder& setDomain(std::string domain) {
    _domain = std::move(domain);
    return *this;
  }

  TransactionBuilder& setName(std::string name) {
    _name = std::move(name);
    return *this;
  }

  TransactionBuilder& setValue(object::BaseObject value) {
    _value = std::move(value);
    return *this;
  }

  object::SimpleAsset build() {

    const auto unsetMembers = enumerateUnsetMembers();
    if (not unsetMembers.empty()) {
      throw exception::transaction::UnsetBuildArgmentsException("object::SimpleAsset", unsetMembers);
    }

    return object::SimpleAsset(_domain, _name, _value);
  }

private:

  std::string enumerateUnsetMembers() {
    std::string ret;
    if (_domain.empty())  ret += " domain";
    if (_name.empty())    ret += " name";
    if (_value.type() == object::BaseObject::Object_type::NONE) ret += " value";
    return ret;
  }

  std::string _domain;
  std::string _name;
  object::BaseObject _value;
};

/*****************************************************************************
  object::Peer
 *****************************************************************************/
template<>
class TransactionBuilder<object::Peer> {
public:

  TransactionBuilder() = default;
  TransactionBuilder(const TransactionBuilder&) = default;
  TransactionBuilder(TransactionBuilder&&) = default;

  TransactionBuilder& setIP(std::string ip) {
    _ip = ip;
    return *this;
  }

  TransactionBuilder& setPublicKey(std::string publicKey) {
    _publicKey = publicKey;
    return *this;
  }

  TransactionBuilder& setTrustScore(double trustScore) {
    _trustScore = trustScore;
    return *this;
  }

  object::Peer build() {

    const auto unsetMembers = enumerateUnsetMembers();
    if (not unsetMembers.empty()) {
      throw exception::transaction::UnsetBuildArgmentsException("object::Peer", unsetMembers);
    }

    return object::Peer(_ip, _publicKey, _trustScore);
  }

private:

  std::string enumerateUnsetMembers() {
    std::string ret;
    if (_ip.empty())        ret += " ip";
    if (_publicKey.empty()) ret += " publicKey";
    if (_trustScore < 0)    ret += " trustScore";
    return ret;
  }

  std::string _ip;
  std::string _publicKey;
  double      _trustScore = -1; // what is the best way to use defaut value?
};

}

#endif