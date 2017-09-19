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

#ifndef IROHA_PB_COMMAND_FACTORY_HPP
#define IROHA_PB_COMMAND_FACTORY_HPP

#include "commands.pb.h"
#include "model/commands/add_asset_quantity.hpp"
#include "model/commands/add_peer.hpp"
#include "model/commands/add_signatory.hpp"
#include "model/commands/create_account.hpp"
#include "model/commands/create_asset.hpp"
#include "model/commands/create_domain.hpp"
#include "model/commands/remove_signatory.hpp"
#include "model/commands/set_permissions.hpp"
#include "model/commands/set_quorum.hpp"
#include "model/commands/transfer_asset.hpp"

namespace iroha {
  namespace model {
    namespace converters {

      /**
       * Converting commands and proto commands
       */
      class PbCommandFactory {
       public:
        // asset quantity
        protocol::AddAssetQuantity serializeAddAssetQuantity(
            const model::AddAssetQuantity &addAssetQuantity);
        model::AddAssetQuantity deserializeAddAssetQuantity(
            const protocol::AddAssetQuantity &addAssetQuantity);

        // add peer
        protocol::AddPeer serializeAddPeer(const model::AddPeer &addPeer);
        model::AddPeer deserializeAddPeer(const protocol::AddPeer &addPeer);

        // add signatory
        protocol::AddSignatory serializeAddSignatory(
            const model::AddSignatory &addSignatory);
        model::AddSignatory deserializeAddSignatory(
            const protocol::AddSignatory &addSignatory);

        // create asset
        protocol::CreateAsset serializeCreateAsset(
            const model::CreateAsset &createAsset);
        model::CreateAsset deserializeCreateAsset(
            const protocol::CreateAsset &createAsset);

        // create account
        protocol::CreateAccount serializeCreateAccount(
            const model::CreateAccount &createAccount);
        model::CreateAccount deserializeCreateAccount(
            const protocol::CreateAccount &createAccount);

        // create domain
        protocol::CreateDomain serializeCreateDomain(
            const model::CreateDomain &createDomain);
        model::CreateDomain deserializeCreateDomain(
            const protocol::CreateDomain &createDomain);

        // remove signatory
        protocol::RemoveSignatory serializeRemoveSignatory(
            const model::RemoveSignatory &removeSignatory);
        model::RemoveSignatory deserializeRemoveSignatory(
            const protocol::RemoveSignatory &removeSignatory);

        // set account permissions
        protocol::SetAccountPermissions serializeSetAccountPermissions(
            const model::SetAccountPermissions &setAccountPermissions);
        model::SetAccountPermissions deserializeSetAccountPermissions(
            const protocol::SetAccountPermissions &setAccountPermissions);

        // set account quorum
        protocol::SetAccountQuorum serializeSetQuorum(
            const model::SetQuorum &setAccountQuorum);
        model::SetQuorum deserializeSetQuorum(
            const protocol::SetAccountQuorum &setAccountQuorum);

        // transfer asset
        protocol::TransferAsset serializeTransferAsset(
            const model::TransferAsset &subtractAssetQuantity);
        model::TransferAsset deserializeTransferAsset(
            const protocol::TransferAsset &subtractAssetQuantity);

        // abstract
        protocol::Command serializeAbstractCommand(
            const model::Command &command);
        std::shared_ptr<model::Command> deserializeAbstractCommand(
            const protocol::Command &command);
      };
    }  // namespace converters
  }    // namespace model
}  // namespace iroha
#endif  // IROHA_PB_COMMAND_FACTORY_HPP
