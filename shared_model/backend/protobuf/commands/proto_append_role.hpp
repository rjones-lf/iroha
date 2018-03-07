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

#ifndef IROHA_PROTO_APPEND_ROLE_HPP
#define IROHA_PROTO_APPEND_ROLE_HPP

#include "interfaces/commands/append_role.hpp"

namespace shared_model {
  namespace proto {

    class AppendRole final : public CopyableProto<interface::AppendRole,
                                                  iroha::protocol::Command,
                                                  AppendRole> {
     public:
      template <typename CommandType>
      explicit AppendRole(CommandType &&command)
          : CopyableProto(std::forward<CommandType>(command)) {}

      AppendRole(const AppendRole &o) : AppendRole(o.proto_) {}

      AppendRole(AppendRole &&o) noexcept : AppendRole(std::move(o.proto_)) {}

      const interface::types::AccountIdType &accountId() const override {
        return append_role_.account_id();
      }

      const interface::types::RoleIdType &roleName() const override {
        return append_role_.role_name();
      }

     private:
      const iroha::protocol::AppendRole &append_role_{proto_->append_role()};
    };

  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_PROTO_APPEND_ROLE_HPP
