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
#ifndef CORE_MODEL_TRANSACTION_BUILDER_REMOVE_MESSAGE_HPP
#define CORE_MODEL_TRANSACTION_BUILDER_REMOVE_MESSAGE_HPP

#include "../transaction_builder_base.hpp"
#include "../../transaction.hpp"
#include "../../type_signatures/remove.hpp"
#include "../../objects/message.hpp"

namespace transaction {

template <>
class TransactionBuilder<type_signatures::Remove<object::Message>> {
 public:
  TransactionBuilder() = default;
  TransactionBuilder(const TransactionBuilder&) = default;
  TransactionBuilder(TransactionBuilder&&) = default;

  TransactionBuilder& setSender(std::string sender) {
    if (_isSetSender) {
      throw std::domain_error(std::string("Duplicate sender in ") +
                              "remove/remove_message_builder_template.hpp");
    }
    _isSetSender = true;
    _sender = std::move(sender);
    return *this;
  }

  TransactionBuilder& setMessage(object::Message object) {
    if (_isSetMessage) {
      throw std::domain_error(std::string("Duplicate ") + "Message" + " in " +
                              "remove/remove_message_builder_template.hpp");
    }
    _isSetMessage = true;
    _message = std::move(object);
    return *this;
  }

  transaction::Transaction build() {
    const auto unsetMembers = enumerateUnsetMembers();
    if (not unsetMembers.empty()) {
      throw exception::transaction::UnsetBuildArgmentsException(
          "Remove<object::Message>", unsetMembers);
    }
    return transaction::Transaction(_sender, command::Remove(_message));
  }

 private:
  std::string enumerateUnsetMembers() {
    std::string ret;
    if (not _isSetSender) ret += std::string(" ") + "sender";
    if (not _isSetMessage) ret += std::string(" ") + "Message";
    return ret;
  }

  std::string _sender;
  object::Message _message;

  bool _isSetSender = false;
  bool _isSetMessage = false;
};
}

#endif
