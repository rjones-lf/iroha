/*
Copyright 2016 Soramitsu Co., Ltd.

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
#ifndef CORE_MODEL_TRANSACTION_BUILDER_ADD_SIMPLE_ASSET_HPP
#define CORE_MODEL_TRANSACTION_BUILDER_ADD_SIMPLE_ASSET_HPP

#include "../transaction_builder_base.hpp"
#include "../../transaction.hpp"
#include "../../type_signatures/add.hpp"
#include "../../objects/simple_asset.hpp"

namespace transaction {

template <>
class TransactionBuilder<type_signatures::Add<object::SimpleAsset>> {
 public:
  TransactionBuilder() = default;
  TransactionBuilder(const TransactionBuilder&) = default;
  TransactionBuilder(TransactionBuilder&&) = default;

  TransactionBuilder& setSender(std::string sender) {
    if (_isSetSender) {
      throw std::domain_error(std::string("Duplicate sender in ") +
                              "add/add_simple_asset_builder_template.hpp");
    }
    _isSetSender = true;
    _sender = std::move(sender);
    return *this;
  }

  TransactionBuilder& setSimpleAsset(object::SimpleAsset object) {
    if (_isSetSimpleAsset) {
      throw std::domain_error(std::string("Duplicate ") + "SimpleAsset" +
                              " in " +
                              "add/add_simple_asset_builder_template.hpp");
    }
    _isSetSimpleAsset = true;
    _simpleAsset = std::move(object);
    return *this;
  }

  transaction::Transaction build() {
    const auto unsetMembers = enumerateUnsetMembers();
    if (not unsetMembers.empty()) {
      throw exception::transaction::UnsetBuildArgmentsException(
          "Add<object::SimpleAsset>", unsetMembers);
    }
    return transaction::Transaction(_sender, command::Add(_simpleAsset));
  }

 private:
  std::string enumerateUnsetMembers() {
    std::string ret;
    if (not _isSetSender) ret += std::string(" ") + "sender";
    if (not _isSetSimpleAsset) ret += std::string(" ") + "SimpleAsset";
    return ret;
  }

  std::string _sender;
  object::SimpleAsset _simpleAsset;

  bool _isSetSender = false;
  bool _isSetSimpleAsset = false;
};
}

#endif
