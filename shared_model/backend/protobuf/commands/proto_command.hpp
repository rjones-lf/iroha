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
#include "commands.pb.h"
#include "interfaces/common_objects/types.hpp"
#include "utils/lazy_initializer.hpp"

namespace shared_model {
  namespace proto {

    class Command final : public CopyableProto<interface::Command,
                                               iroha::protocol::Command,
                                               Command> {
     public:
      /// type of proto variant
      using ProtoCommandVariantType = boost::variant<AddAssetQuantity,
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
      explicit Command(CommandType &&command);

      Command(const Command &o);

      Command(Command &&o) noexcept;

      const CommandVariantType &get() const override;

     private:
      /// lazy variant shortcut
      template <typename T>
      using Lazy = detail::LazyInitializer<T>;

      using LazyVariantType = Lazy<ProtoCommandVariantType>;

      // lazy
      const LazyVariantType variant_;

      const Lazy<CommandVariantType> ivariant_;
    };
  }  // namespace proto
}  // namespace shared_model

namespace boost {
  extern template class variant<shared_model::proto::AddAssetQuantity,
                                shared_model::proto::AddPeer,
                                shared_model::proto::AddSignatory,
                                shared_model::proto::AppendRole,
                                shared_model::proto::CreateAccount,
                                shared_model::proto::CreateAsset,
                                shared_model::proto::CreateDomain,
                                shared_model::proto::CreateRole,
                                shared_model::proto::DetachRole,
                                shared_model::proto::GrantPermission,
                                shared_model::proto::RemoveSignatory,
                                shared_model::proto::RevokePermission,
                                shared_model::proto::SetAccountDetail,
                                shared_model::proto::SetQuorum,
                                shared_model::proto::SubtractAssetQuantity,
                                shared_model::proto::TransferAsset>;

}  // namespace boost

#endif  // IROHA_SHARED_MODEL_PROTO_COMMAND_HPP
