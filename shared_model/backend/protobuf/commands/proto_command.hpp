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

#ifndef IROHA_SHARED_MODEL_PROTO_COMMAND_HPP
#define IROHA_SHARED_MODEL_PROTO_COMMAND_HPP

#include "interfaces/commands/command.hpp"

#include "backend/protobuf/commands/proto_add_asset_quantity.hpp"
#include "backend/protobuf/commands/proto_add_peer.hpp"
#include "backend/protobuf/commands/proto_add_signatory.hpp"
#include "backend/protobuf/commands/proto_append_role.hpp"
#include "backend/protobuf/commands/proto_create_account.hpp"
#include "backend/protobuf/commands/proto_create_asset.hpp"
#include "backend/protobuf/commands/proto_create_domain.hpp"
#include "backend/protobuf/commands/proto_create_role.hpp"
#include "backend/protobuf/commands/proto_detach_role.hpp"
#include "backend/protobuf/commands/proto_grant_permission.hpp"
#include "backend/protobuf/commands/proto_remove_signatory.hpp"
#include "backend/protobuf/commands/proto_revoke_permission.hpp"
#include "backend/protobuf/commands/proto_set_account_detail.hpp"
#include "backend/protobuf/commands/proto_set_quorum.hpp"
#include "backend/protobuf/commands/proto_subtract_asset_quantity.hpp"
#include "backend/protobuf/commands/proto_transfer_asset.hpp"
#include "backend/protobuf/common_objects/trivial_proto.hpp"
#include "backend/protobuf/util.hpp"
#include "commands.pb.h"
#include "interfaces/common_objects/types.hpp"
#include "utils/lazy_initializer.hpp"
#include "utils/variant_deserializer.hpp"

template <typename... T, typename Archive>
auto loadCommand(Archive &&ar) {
  int which = ar.GetDescriptor()->FindFieldByNumber(ar.command_case())->index();
  return shared_model::detail::variant_impl<T...>::
      template load<shared_model::interface::Command::CommandVariantType>(
          std::forward<Archive>(ar), which);
}

namespace shared_model {
  namespace proto {

    class Command final : public CopyableProto<interface::Command,
                                               iroha::protocol::Command,
                                               Command> {
     private:
      /// polymorphic wrapper type shortcut
      template <typename... Value>
      using wrap = boost::variant<detail::PolymorphicWrapper<Value>...>;

      /// lazy variant shortcut
      template <typename T>
      using Lazy = detail::LazyInitializer<T>;

      using LazyVariantType = Lazy<CommandVariantType>;

     public:
      /// type of proto variant
      using ProtoCommandVariantType = wrap<AddAssetQuantity,
                                           AddPeer,
                                           AddSignatory,
                                           AppendRole,
                                           CreateAccount,
                                           CreateAsset,
                                           CreateDomain,
                                           CreateRole,
                                           DetachRole,
                                           GrantPermission,
                                           RemoveSignatory,
                                           RevokePermission,
                                           SetAccountDetail,
                                           SetQuorum,
                                           SubtractAssetQuantity,
                                           TransferAsset>;

      /// list of types in proto variant
      using ProtoCommandListType = ProtoCommandVariantType::types;

      template <typename CommandType>
      explicit Command(CommandType &&command)
          : CopyableProto(std::forward<CommandType>(command)) {}

      Command(const Command &o) : Command(o.proto_) {}

      Command(Command &&o) noexcept : Command(std::move(o.proto_)) {}

      const CommandVariantType &get() const override {
        return *variant_;
      }

     private:
      // lazy
      const LazyVariantType variant_{
          [this] { return loadCommand<ProtoCommandListType>(*proto_); }};
    };
  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_SHARED_MODEL_PROTO_COMMAND_HPP
