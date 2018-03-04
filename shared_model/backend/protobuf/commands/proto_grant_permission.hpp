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

#ifndef IROHA_PROTO_GRANT_PERMISSION_HPP
#define IROHA_PROTO_GRANT_PERMISSION_HPP

#include "interfaces/commands/grant_permission.hpp"

namespace shared_model {
  namespace proto {

    class GrantPermission final
        : public CopyableProto<interface::GrantPermission,
                               iroha::protocol::Command,
                               GrantPermission> {
     public:
      template <typename CommandType>
      explicit GrantPermission(CommandType &&command)
          : CopyableProto(std::forward<CommandType>(command)) {}

      GrantPermission(const GrantPermission &o) : GrantPermission(o.proto_) {}

      GrantPermission(GrantPermission &&o) noexcept
          : GrantPermission(std::move(o.proto_)) {}

      const interface::types::AccountIdType &accountId() const override {
        return grant_permission_.account_id();
      }

      const interface::types::PermissionNameType &permissionName()
          const override {
        return iroha::protocol::GrantablePermission_Name(
            grant_permission_.permission());
      }

     private:
      const iroha::protocol::GrantPermission &grant_permission_{
          proto_->grant_permission()};
    };

  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_PROTO_GRANT_PERMISSION_HPP
