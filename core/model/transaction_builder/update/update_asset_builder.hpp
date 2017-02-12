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
#ifndef CORE_MODEL_TRANSACTION_BUILDER_UPDATE_ASSET_HPP
#define CORE_MODEL_TRANSACTION_BUILDER_UPDATE_ASSET_HPP

#include "../transaction_builder_base.hpp"
#include "../../transaction.hpp"
#include "../../type_signatures/update.hpp"
#include "../../objects/asset.hpp"

namespace transaction {

template <>
class TransactionBuilder<type_signatures::Update<object::Asset>> {
 public:
  TransactionBuilder() = default;
  TransactionBuilder(const TransactionBuilder&) = default;
  TransactionBuilder(TransactionBuilder&&) = default;

  TransactionBuilder& setSender(std::string sender) {
    if (_isSetSender) {
      throw std::domain_error(std::string("Duplicate sender in ") +
                              "update/update_asset_builder_template.hpp");
    }
    _isSetSender = true;
    _sender = std::move(sender);
    return *this;
  }

  TransactionBuilder& setAsset(object::Asset object) {
    if (_isSetAsset) {
      throw std::domain_error(std::string("Duplicate ") + "Asset" + " in " +
                              "update/update_asset_builder_template.hpp");
    }
    _isSetAsset = true;
    _asset = std::move(object);
    return *this;
  }

  transaction::Transaction build() {
    const auto unsetMembers = enumerateUnsetMembers();
    if (not unsetMembers.empty()) {
      throw exception::transaction::UnsetBuildArgmentsException(
          "Update<object::Asset>", unsetMembers);
    }
    return transaction::Transaction(_sender, command::Update(_asset));
  }

 private:
  std::string enumerateUnsetMembers() {
    std::string ret;
    if (not _isSetSender) ret += std::string(" ") + "sender";
    if (not _isSetAsset) ret += std::string(" ") + "Asset";
    return ret;
  }

  std::string _sender;
  object::Asset _asset;

  bool _isSetSender = false;
  bool _isSetAsset = false;
};
}

#endif
