/**
 * Copyright Soramitsu Co., Ltd. 2017 All Rights Reserved.
 * http://soramitsu.co.jp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef IROHA_PROTO_REMOVE_SIGNATORY_HPP
#define IROHA_PROTO_REMOVE_SIGNATORY_HPP

#include "interfaces/commands/remove_signatory.hpp"

namespace shared_model {
  namespace proto {

    class RemoveSignatory final
        : public CopyableProto<interface::RemoveSignatory,
                               iroha::protocol::Command,
                               RemoveSignatory> {
     public:
      template <typename CommandType>
      explicit RemoveSignatory(CommandType &&command)
          : CopyableProto(std::forward<CommandType>(command)) {}

      RemoveSignatory(const RemoveSignatory &o) : RemoveSignatory(o.proto_) {}

      RemoveSignatory(RemoveSignatory &&o) noexcept
          : RemoveSignatory(std::move(o.proto_)) {}

      const interface::types::AccountIdType &accountId() const override {
        return remove_signatory_.account_id();
      }

      const interface::types::PubkeyType &pubkey() const override {
        return *pubkey_;
      }

     private:
      // lazy
      template <typename Value>
      using Lazy = detail::LazyInitializer<Value>;

      const iroha::protocol::RemoveSignatory &remove_signatory_{
          proto_->remove_sign()};

      const Lazy<interface::types::PubkeyType> pubkey_{[this] {
        return interface::types::PubkeyType(remove_signatory_.public_key());
      }};
    };

  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_PROTO_REMOVE_SIGNATORY_HPP
