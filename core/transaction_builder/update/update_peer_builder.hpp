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
#ifndef CORE_MODEL_TRANSACTION_BUILDER_UPDATE_PEER_HPP
#define CORE_MODEL_TRANSACTION_BUILDER_UPDATE_PEER_HPP

#include <infra/protobuf/api.pb.h>
#include <util/exception.hpp>
#include "../transaction_builder_base.hpp"
#include "../type_signatures/commands/update.hpp"
#include "../type_signatures/objects.hpp"

namespace txbuilder {

  template<>
  class TransactionBuilder<type_signatures::Update<type_signatures::Peer>> {
  public:
    TransactionBuilder() = default;

    TransactionBuilder(const TransactionBuilder &) = default;

    TransactionBuilder(TransactionBuilder &&) = default;

    TransactionBuilder &setSenderPublicKey(std::string sender) {
      if (_isSetSenderPublicKey) {
        throw exception::txbuilder::DuplicateSetArgmentException(
            "Update<Peer>", "senderPublicKey");
      }
      _isSetSenderPublicKey = true;
      _senderPublicKey = std::move(sender);
      return *this;
    }

    TransactionBuilder &setPeer(Api::Peer object) {
      if (_isSetPeer) {
        throw exception::txbuilder::DuplicateSetArgmentException("Update<Peer>",
                                                                 "Peer");
      }
      _isSetPeer = true;
      _peer = std::move(object);
      return *this;
    }

    Api::Transaction build() {
      const auto unsetMembers = enumerateUnsetMembers();
      if (not unsetMembers.empty()) {
        throw exception::txbuilder::UnsetBuildArgmentsException("Update<Peer>",
                                                                unsetMembers);
      }
      Api::Transaction ret;
      ret.set_senderpubkey(_senderPublicKey);
      ret.set_type("Update");
      auto ptr = std::make_unique<Api::Peer>();
      ptr->CopyFrom(_peer);
      ret.set_allocated_peer(ptr.release());
      return ret;
    }

  private:
    std::string enumerateUnsetMembers() {
      std::string ret;
      if (not _isSetSenderPublicKey) ret += std::string(" ") + "sender";
      if (not _isSetPeer) ret += std::string(" ") + "Peer";
      return ret;
    }

    std::string _senderPublicKey;
    Api::Peer _peer;

    bool _isSetSenderPublicKey = false;
    bool _isSetPeer = false;
  };
}

#endif
