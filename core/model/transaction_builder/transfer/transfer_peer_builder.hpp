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
#ifndef CORE_MODEL_TRANSACTION_BUILDER_TRANSFER_PEER_HPP
#define CORE_MODEL_TRANSACTION_BUILDER_TRANSFER_PEER_HPP

#include "../transaction_builder_base.hpp"
#include "../../transaction.hpp"
#include "../../type_signatures/transfer.hpp"
#include "../../objects/peer.hpp"

namespace transaction {

template <>
class TransactionBuilder<type_signatures::Transfer<object::Peer>> {
 public:
  TransactionBuilder() = default;
  TransactionBuilder(const TransactionBuilder&) = default;
  TransactionBuilder(TransactionBuilder&&) = default;

  TransactionBuilder& setSender(std::string sender) {
    if (_isSetSender) {
      throw std::domain_error(std::string("Duplicate sender in ") +
                              "transfer/transfer_peer_builder_template.hpp");
    }
    _isSetSender = true;
    _sender = std::move(sender);
    return *this;
  }

  TransactionBuilder& setReceiverPublicKey(std::string receiverPublicKey) {
    if (_isSetReceiverPublicKey) {
      throw std::domain_error(std::string("Duplicate receiverPublicKey in ") +
                              "transfer/transfer_peer_builder_template.hpp");
    }
    _isSetReceiverPublicKey = true;
    _receiverPublicKey = std::move(receiverPublicKey);
    return *this;
  }

  TransactionBuilder& setPeer(object::Peer object) {
    if (_isSetPeer) {
      throw std::domain_error(std::string("Duplicate ") + "Peer" + " in " +
                              "transfer/transfer_peer_builder_template.hpp");
    }
    _isSetPeer = true;
    _peer = std::move(object);
    return *this;
  }

  transaction::Transaction build() {
    const auto unsetMembers = enumerateUnsetMembers();
    if (not unsetMembers.empty()) {
      throw exception::transaction::UnsetBuildArgmentsException(
          "Transfer<object::Peer>", unsetMembers);
    }
    return transaction::Transaction(
        _sender, command::Transfer(_peer, _receiverPublicKey));
  }

 private:
  std::string enumerateUnsetMembers() {
    std::string ret;
    if (not _isSetSender) ret += std::string(" ") + "sender";
    if (not _isSetReceiverPublicKey)
      ret += std::string(" ") + "receiverPublicKey";
    if (not _isSetPeer) ret += std::string(" ") + "Peer";
    return ret;
  }

  std::string _sender;
  std::string _receiverPublicKey;
  object::Peer _peer;

  bool _isSetSender = false;
  bool _isSetReceiverPublicKey = false;
  bool _isSetPeer = false;
};
}

#endif
