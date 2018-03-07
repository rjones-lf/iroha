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

#ifndef IROHA_PROTO_SET_QUORUM_HPP
#define IROHA_PROTO_SET_QUORUM_HPP

#include "interfaces/commands/set_quorum.hpp"

namespace shared_model {
  namespace proto {
    class SetQuorum final : public CopyableProto<interface::SetQuorum,
                                                 iroha::protocol::Command,
                                                 SetQuorum> {
     public:
      template <typename CommandType>
      explicit SetQuorum(CommandType &&command)
          : CopyableProto(std::forward<CommandType>(command)) {}

      SetQuorum(const SetQuorum &o) : SetQuorum(o.proto_) {}

      SetQuorum(SetQuorum &&o) noexcept : SetQuorum(std::move(o.proto_)) {}

      const interface::types::AccountIdType &accountId() const override {
        return set_quorum_.account_id();
      }

      const interface::types::QuorumType &newQuorum() const override {
        return *new_quorum_;
      }

     private:
      // lazy
      template <typename T>
      using Lazy = detail::LazyInitializer<T>;

      const iroha::protocol::SetAccountQuorum &set_quorum_{
          proto_->set_quorum()};

      const Lazy<const interface::types::QuorumType> new_quorum_{
          [this] { return set_quorum_.quorum(); }};
    };

  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_PROTO_SET_QUORUM_HPP
