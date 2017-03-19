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
#ifndef CORE_MODEL_TRANSACTION_BUILDER_ADD_ACCOUNT_HPP
#define CORE_MODEL_TRANSACTION_BUILDER_ADD_ACCOUNT_HPP

#include <infra/protobuf/api.pb.h>
#include <util/exception.hpp>
#include "../transaction_builder_base.hpp"
#include "../type_signatures/commands/add.hpp"
#include "../type_signatures/objects.hpp"

namespace txbuilder {

  template<>
  class TransactionBuilder<type_signatures::Add<type_signatures::Account>> {
  public:
    TransactionBuilder() = default;

    TransactionBuilder(const TransactionBuilder &) = default;

    TransactionBuilder(TransactionBuilder &&) = default;

    TransactionBuilder &setSenderPublicKey(std::string senderPublicKey) {
      if (_isSetSenderPublicKey) {
        throw exception::txbuilder::DuplicateSetArgmentException(
            "Add<Account>", "senderPublicKey");
      }
      _isSetSenderPublicKey = true;
      _senderPublicKey = std::move(senderPublicKey);
      return *this;
    }

    TransactionBuilder &setAccount(Api::Account object) {
      if (_isSetAccount) {
        throw exception::txbuilder::DuplicateSetArgmentException("Add<Account>",
                                                                 "Account");
      }
      _isSetAccount = true;
      _account = std::move(object);
      return *this;
    }

    Api::Transaction build() {
      const auto unsetMembers = enumerateUnsetMembers();
      if (not unsetMembers.empty()) {
        throw exception::txbuilder::UnsetBuildArgmentsException("Add<Account>",
                                                                unsetMembers);
      }
      Api::Transaction ret;
      ret.set_senderpubkey(_senderPublicKey);
      ret.set_type("Add");
      auto ptr = std::make_unique<Api::Account>();
      ptr->CopyFrom(_account);
      ret.set_allocated_account(ptr.release());
      return ret;
    }

  private:
    std::string enumerateUnsetMembers() {
      std::string ret;
      if (not _isSetSenderPublicKey) ret += std::string(" ") + "sender";
      if (not _isSetAccount) ret += std::string(" ") + "Account";
      return ret;
    }

    std::string _senderPublicKey;
    Api::Account _account;

    bool _isSetSenderPublicKey = false;
    bool _isSetAccount = false;
  };
}

#endif
