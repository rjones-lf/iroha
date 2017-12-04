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

#ifndef IROHA_COMMAND_REGISTRATION_HPP
#define IROHA_COMMAND_REGISTRATION_HPP

#include "common/class_handler.hpp"

// ----------| commands |----------
#include "model/commands/add_asset_quantity.hpp"
#include "model/commands/subtract_asset_quantity.hpp"
#include "model/commands/add_peer.hpp"
#include "model/commands/add_signatory.hpp"
#include "model/commands/create_account.hpp"
#include "model/commands/create_asset.hpp"
#include "model/commands/create_domain.hpp"
#include "model/commands/remove_signatory.hpp"
#include "model/commands/set_quorum.hpp"
#include "model/commands/transfer_asset.hpp"

#include "model/commands/append_role.hpp"
#include "model/commands/create_role.hpp"
#include "model/commands/grant_permission.hpp"
#include "model/commands/revoke_permission.hpp"
#include "model/commands/set_account_detail.hpp"

/**
 * File contains registration for all command subclasses
 */

namespace iroha {
  namespace model {

    class CommandRegistry {
     public:
      CommandRegistry() {
        command_handler.register_type(typeid(AddAssetQuantity));
        command_handler.register_type(typeid(SubtractAssetQuantity));
        command_handler.register_type(typeid(AddPeer));
        command_handler.register_type(typeid(AddSignatory));
        command_handler.register_type(typeid(CreateAccount));
        command_handler.register_type(typeid(CreateAsset));
        command_handler.register_type(typeid(CreateDomain));
        command_handler.register_type(typeid(RemoveSignatory));
        command_handler.register_type(typeid(SetQuorum));
        command_handler.register_type(typeid(TransferAsset));
        command_handler.register_type(typeid(AppendRole));
        command_handler.register_type(typeid(CreateRole));
        command_handler.register_type(typeid(GrantPermission));
        command_handler.register_type(typeid(RevokePermission));
        command_handler.register_type(typeid(SetAccountDetail));
      }

      ClassHandler command_handler{};
    };

  } // namespace model
} // namespace iroha

#endif //IROHA_COMMAND_REGISTRATION_HPP
