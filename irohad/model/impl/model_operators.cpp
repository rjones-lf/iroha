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
#include "model/block.hpp"
#include "model/commands/add_asset_quantity.hpp"
#include "model/commands/add_peer.hpp"
#include "model/commands/add_signatory.hpp"
#include "model/commands/append_role.hpp"
#include "model/commands/create_account.hpp"
#include "model/commands/create_asset.hpp"
#include "model/commands/create_domain.hpp"
#include "model/commands/create_role.hpp"
#include "model/commands/grant_permission.hpp"
#include "model/commands/remove_signatory.hpp"
#include "model/commands/revoke_permission.hpp"
#include "model/commands/set_account_detail.hpp"
#include "model/commands/set_quorum.hpp"
#include "model/commands/subtract_asset_quantity.hpp"
#include "model/commands/transfer_asset.hpp"

namespace iroha {
  namespace model {

    bool Command::operator!=(const Command &rhs) const {
      return not operator==(rhs);
    }

    bool Block::operator!=(const Block &rhs) const {
      return not operator==(rhs);
    }

    bool Proposal::operator!=(const Proposal &rhs) const {
      return not operator==(rhs);
    }

    bool Transaction::operator!=(const Transaction &rhs) const {
      return not operator==(rhs);
    }

    bool Signature::operator!=(const Signature &rhs) const {
      return not operator==(rhs);
    }

    bool AppendRole::operator==(const Command &command) const {
      if (! instanceof <AppendRole>(command)) return false;
      auto cmd = static_cast<const AppendRole &>(command);
      return cmd.account_id == account_id and cmd.role_name == role_name;
    }

    bool CreateRole::operator==(const Command &command) const {
      if (! instanceof <CreateRole>(command)) return false;
      auto cmd = static_cast<const CreateRole &>(command);
      return cmd.role_name == role_name and cmd.permissions == permissions;
    }

    bool GrantPermission::operator==(const Command &command) const {
      if (! instanceof <GrantPermission>(command)) return false;
      auto cmd = static_cast<const GrantPermission &>(command);
      return cmd.account_id == account_id
          and cmd.permission_name == permission_name;
    }

    bool RevokePermission::operator==(const Command &command) const {
      if (! instanceof <RevokePermission>(command)) return false;
      auto cmd = static_cast<const RevokePermission &>(command);
      return cmd.account_id == account_id
          and cmd.permission_name == permission_name;
    }

    /* AddAssetQuantity */
    bool AddAssetQuantity::operator==(const Command &command) const {
      if (! instanceof <AddAssetQuantity>(command)) return false;
      auto add_asset_quantity = static_cast<const AddAssetQuantity &>(command);
      return add_asset_quantity.account_id == account_id
          && add_asset_quantity.asset_id == asset_id
          && add_asset_quantity.amount == amount;
    }

    /* SubtractAssetQuantity */
    bool SubtractAssetQuantity::operator==(const Command &command) const {
      if (! instanceof <SubtractAssetQuantity>(command)) return false;
      auto subtract_asset_quantity = static_cast<const SubtractAssetQuantity &>(command);
      return subtract_asset_quantity.account_id == account_id
             && subtract_asset_quantity.asset_id == asset_id
             && subtract_asset_quantity.amount == amount;
    }

    /* AddPeer */
    bool AddPeer::operator==(const Command &command) const {
      if (! instanceof <AddPeer>(command)) return false;
      auto add_peer = static_cast<const AddPeer &>(command);
      return add_peer.peer_key == peer_key && add_peer.address == address;
    }

    /* AddSignatory */
    bool AddSignatory::operator==(const Command &command) const {
      if (! instanceof <AddSignatory>(command)) return false;
      auto add_signatory = static_cast<const AddSignatory &>(command);
      return add_signatory.account_id == account_id
          && add_signatory.pubkey == pubkey;
    }

    /* CreateAccount */
    bool CreateAccount::operator==(const Command &command) const {
      if (! instanceof <CreateAccount>(command)) return false;
      auto create_account = static_cast<const CreateAccount &>(command);
      return create_account.pubkey == pubkey
          && create_account.domain_id == domain_id
          && create_account.account_name == account_name
          && create_account.json_data == create_account.json_data;
    }

    /* SetAccountDetail */
    bool SetAccountDetail::operator==(const Command &rhs) const {
      if (! instanceof <SetAccountDetail>(rhs)) {
        return false;
      }
      auto setAccountDetail = static_cast<const SetAccountDetail &>(rhs);
      return setAccountDetail.account_id == account_id
          and setAccountDetail.key == key and setAccountDetail.value == value;
    }

    /* CreateAsset */
    bool CreateAsset::operator==(const Command &command) const {
      if (! instanceof <CreateAsset>(command)) return false;
      auto create_asset = static_cast<const CreateAsset &>(command);
      return create_asset.domain_id == domain_id
          && create_asset.precision == precision
          && create_asset.asset_name == asset_name;
    }

    /* Create domain */
    bool CreateDomain::operator==(const Command &command) const {
      if (! instanceof <CreateDomain>(command)) return false;
      auto create_domain = static_cast<const CreateDomain &>(command);
      return create_domain.domain_id == domain_id
          && create_domain.user_default_role == user_default_role;
    }

    /* Remove signatory */
    bool RemoveSignatory::operator==(const Command &command) const {
      if (! instanceof <RemoveSignatory>(command)) return false;
      auto remove_signatory = static_cast<const RemoveSignatory &>(command);
      return remove_signatory.pubkey == pubkey
          && remove_signatory.account_id == account_id;
    }

    /* Set Quorum*/
    bool SetQuorum::operator==(const Command &command) const {
      if (! instanceof <SetQuorum>(command)) return false;
      auto set_quorum = static_cast<const SetQuorum &>(command);
      return set_quorum.account_id == account_id
          && set_quorum.new_quorum == new_quorum;
    }

    /* Transfer Asset */
    bool TransferAsset::operator==(const Command &command) const {
      if (! instanceof <TransferAsset>(command)) return false;
      auto transfer_asset = static_cast<const TransferAsset &>(command);
      return transfer_asset.asset_id == asset_id
          && transfer_asset.amount == amount
          && transfer_asset.src_account_id == src_account_id
          && transfer_asset.dest_account_id == dest_account_id
          && transfer_asset.description == description;
    }

    /* Signature */
    bool Signature::operator==(const Signature &rhs) const {
      return rhs.pubkey == pubkey && rhs.signature == signature;
    }

    /* Transaction */
    bool Transaction::operator==(const Transaction &rhs) const {
      return std::equal(commands.begin(),
                        commands.end(),
                        rhs.commands.begin(),
                        rhs.commands.end(),
                        [](const auto &i, const auto &j) { return *i == *j; })
          && rhs.tx_counter == tx_counter && rhs.signatures == signatures
          && rhs.created_ts == created_ts;
    }

    /* Proposal */
    bool Proposal::operator==(const Proposal &rhs) const {
      return rhs.height == height and rhs.transactions == transactions;
    }

    /* Block */
    bool Block::operator==(const Block &rhs) const {
      return rhs.hash == hash && rhs.height == height
          && rhs.prev_hash == prev_hash && rhs.txs_number == txs_number
          && rhs.merkle_root == merkle_root && rhs.sigs == sigs
          && rhs.transactions == transactions && rhs.created_ts == created_ts
          && rhs.hash == hash;
    }

  }  // namespace model
}  // namespace iroha
