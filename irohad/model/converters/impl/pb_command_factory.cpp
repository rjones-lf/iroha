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

#include "model/converters/pb_command_factory.hpp"

#include <string>

namespace iroha {
  namespace model {
    namespace converters {

      // asset quantity
      protocol::AddAssetQuantity PbCommandFactory::serializeAddAssetQuantity(
          const model::AddAssetQuantity &add_asset_quantity) {
        protocol::AddAssetQuantity pb_add_asset_quantity;
        pb_add_asset_quantity.set_account_id(add_asset_quantity.account_id);
        pb_add_asset_quantity.set_asset_id(add_asset_quantity.asset_id);
        auto amount = pb_add_asset_quantity.mutable_amount();
        amount->set_integer_part(add_asset_quantity.amount.int_part);
        amount->set_fractial_part(add_asset_quantity.amount.frac_part);
        return pb_add_asset_quantity;
      }

      model::AddAssetQuantity PbCommandFactory::deserializeAddAssetQuantity(
          const protocol::AddAssetQuantity &pb_add_asset_quantity) {
        model::AddAssetQuantity add_asset_quantity;
        add_asset_quantity.account_id = pb_add_asset_quantity.account_id();
        add_asset_quantity.asset_id = pb_add_asset_quantity.asset_id();
        Amount amount;
        amount.int_part = pb_add_asset_quantity.amount().integer_part();
        amount.frac_part = pb_add_asset_quantity.amount().fractial_part();
        add_asset_quantity.amount = amount;

        return add_asset_quantity;
      }

      // add peer
      protocol::AddPeer PbCommandFactory::serializeAddPeer(
          const model::AddPeer &add_peer) {
        protocol::AddPeer pb_add_peer;
        pb_add_peer.set_address(add_peer.address);
        pb_add_peer.set_peer_key(add_peer.peer_key.data(),
                                 add_peer.peer_key.size());
        return pb_add_peer;
      }
      model::AddPeer PbCommandFactory::deserializeAddPeer(
          const protocol::AddPeer &pb_add_peer) {
        model::AddPeer add_peer;
        add_peer.address = pb_add_peer.address();
        std::copy(pb_add_peer.peer_key().begin(), pb_add_peer.peer_key().end(),
                  add_peer.peer_key.begin());
        return add_peer;
      }

      // add signatory
      protocol::AddSignatory PbCommandFactory::serializeAddSignatory(
          const model::AddSignatory &add_signatory) {
        protocol::AddSignatory pb_add_signatory;
        pb_add_signatory.set_account_id(add_signatory.account_id);
        pb_add_signatory.set_public_key(add_signatory.pubkey.data(),
                                        add_signatory.pubkey.size());
        return pb_add_signatory;
      }
      model::AddSignatory PbCommandFactory::deserializeAddSignatory(
          const protocol::AddSignatory &pb_add_signatory) {
        model::AddSignatory add_signatory;
        add_signatory.account_id = pb_add_signatory.account_id();
        std::copy(pb_add_signatory.public_key().begin(),
                  pb_add_signatory.public_key().end(),
                  add_signatory.pubkey.begin());
        return add_signatory;
      }

      // create asset
      protocol::CreateAsset PbCommandFactory::serializeCreateAsset(
          const model::CreateAsset &create_asset) {
        protocol::CreateAsset pb_create_asset;
        pb_create_asset.set_asset_name(create_asset.asset_name);
        pb_create_asset.set_domain_id(create_asset.domain_id);
        pb_create_asset.set_precision(create_asset.precision);
        return pb_create_asset;
      }

      model::CreateAsset PbCommandFactory::deserializeCreateAsset(
          const protocol::CreateAsset &pb_create_asset) {
        model::CreateAsset create_asset;
        create_asset.asset_name = pb_create_asset.asset_name();
        create_asset.domain_id = pb_create_asset.domain_id();
        create_asset.precision =
            static_cast<uint8_t>(pb_create_asset.precision());
        return create_asset;
      }

      // create account
      protocol::CreateAccount PbCommandFactory::serializeCreateAccount(
          const model::CreateAccount &create_account) {
        protocol::CreateAccount pb_create_account;
        pb_create_account.set_account_name(create_account.account_name);
        pb_create_account.set_domain_id(create_account.domain_id);
        pb_create_account.set_main_pubkey(create_account.pubkey.data(),
                                          create_account.pubkey.size());
        return pb_create_account;
      }
      model::CreateAccount PbCommandFactory::deserializeCreateAccount(
          const protocol::CreateAccount &pb_create_account) {
        model::CreateAccount create_account;
        create_account.account_name = pb_create_account.account_name();
        create_account.domain_id = pb_create_account.domain_id();
        std::copy(pb_create_account.main_pubkey().begin(),
                  pb_create_account.main_pubkey().end(),
                  create_account.pubkey.begin());
        return create_account;
      }

      // create domain
      protocol::CreateDomain PbCommandFactory::serializeCreateDomain(
          const model::CreateDomain &create_domain) {
        protocol::CreateDomain pb_create_domain;
        pb_create_domain.set_domain_name(create_domain.domain_name);
        return pb_create_domain;
      }

      model::CreateDomain PbCommandFactory::deserializeCreateDomain(
          const protocol::CreateDomain &pb_create_domain) {
        model::CreateDomain create_domain;
        create_domain.domain_name = pb_create_domain.domain_name();
        return create_domain;
      }

      // remove signatory
      protocol::RemoveSignatory PbCommandFactory::serializeRemoveSignatory(
          const model::RemoveSignatory &remove_signatory) {
        protocol::RemoveSignatory pb_remove_signatory;
        pb_remove_signatory.set_account_id(remove_signatory.account_id);
        pb_remove_signatory.set_public_key(remove_signatory.pubkey.data(),
                                           remove_signatory.pubkey.size());
        return pb_remove_signatory;
      }

      model::RemoveSignatory PbCommandFactory::deserializeRemoveSignatory(
          const protocol::RemoveSignatory &pb_remove_signatory) {
        model::RemoveSignatory remove_signatory;
        remove_signatory.account_id = pb_remove_signatory.account_id();
        std::copy(pb_remove_signatory.public_key().begin(),
                  pb_remove_signatory.public_key().end(),
                  remove_signatory.pubkey.begin());
        return remove_signatory;
      }

      // set account permissions
      protocol::SetAccountPermissions
      PbCommandFactory::serializeSetAccountPermissions(
          const model::SetAccountPermissions &set_account_permissions) {
        protocol::SetAccountPermissions pb_set_account_permissions;
        pb_set_account_permissions.set_account_id(
            set_account_permissions.account_id);
        auto permissions = pb_set_account_permissions.mutable_permissions();
        permissions->set_issue_assets(
            set_account_permissions.new_permissions.issue_assets);
        permissions->set_create_assets(
            set_account_permissions.new_permissions.create_assets);
        permissions->set_create_accounts(
            set_account_permissions.new_permissions.create_accounts);
        permissions->set_create_domains(
            set_account_permissions.new_permissions.create_domains);
        permissions->set_read_all_accounts(
            set_account_permissions.new_permissions.read_all_accounts);
        permissions->set_add_signatory(
            set_account_permissions.new_permissions.add_signatory);
        permissions->set_remove_signatory(
            set_account_permissions.new_permissions.remove_signatory);
        permissions->set_set_quorum(
            set_account_permissions.new_permissions.set_quorum);
        permissions->set_can_transfer(
            set_account_permissions.new_permissions.can_transfer);
        return pb_set_account_permissions;
      }

      model::SetAccountPermissions
      PbCommandFactory::deserializeSetAccountPermissions(
          const protocol::SetAccountPermissions &pb_set_account_permissions) {
        model::SetAccountPermissions set_account_permissions;
        set_account_permissions.account_id =
            pb_set_account_permissions.account_id();
        set_account_permissions.new_permissions.issue_assets =
            pb_set_account_permissions.permissions().issue_assets();
        set_account_permissions.new_permissions.create_assets =
            pb_set_account_permissions.permissions().create_assets();
        set_account_permissions.new_permissions.create_accounts =
            pb_set_account_permissions.permissions().create_accounts();
        set_account_permissions.new_permissions.create_domains =
            pb_set_account_permissions.permissions().create_domains();
        set_account_permissions.new_permissions.read_all_accounts =
            pb_set_account_permissions.permissions().read_all_accounts();
        set_account_permissions.new_permissions.add_signatory =
            pb_set_account_permissions.permissions().add_signatory();
        set_account_permissions.new_permissions.remove_signatory =
            pb_set_account_permissions.permissions().remove_signatory();
        set_account_permissions.new_permissions.set_permissions =
            pb_set_account_permissions.permissions().set_permissions();
        set_account_permissions.new_permissions.set_quorum =
            pb_set_account_permissions.permissions().set_quorum();
        set_account_permissions.new_permissions.can_transfer =
            pb_set_account_permissions.permissions().can_transfer();
        return set_account_permissions;
      }

      // set account quorum
      protocol::SetAccountQuorum PbCommandFactory::serializeSetQuorum(
          const model::SetQuorum &set_account_quorum) {
        protocol::SetAccountQuorum pb_set_account_quorum;
        pb_set_account_quorum.set_account_id(set_account_quorum.account_id);
        pb_set_account_quorum.set_quorum(set_account_quorum.new_quorum);
        return pb_set_account_quorum;
      }
      model::SetQuorum PbCommandFactory::deserializeSetQuorum(
          const protocol::SetAccountQuorum &pb_set_account_quorum) {
        model::SetQuorum set_quorum;
        set_quorum.account_id = pb_set_account_quorum.account_id();
        set_quorum.new_quorum = pb_set_account_quorum.quorum();
        return set_quorum;
      }

      // transfer asset
      protocol::TransferAsset PbCommandFactory::serializeTransferAsset(
          const model::TransferAsset &transfer_asset) {
        protocol::TransferAsset pb_transfer_asset;
        pb_transfer_asset.set_src_account_id(transfer_asset.src_account_id);
        pb_transfer_asset.set_dest_account_id(transfer_asset.dest_account_id);
        pb_transfer_asset.set_asset_id(transfer_asset.asset_id);
        pb_transfer_asset.set_description(transfer_asset.description);
        auto amount = pb_transfer_asset.mutable_amount();
        amount->set_integer_part(transfer_asset.amount.int_part);
        amount->set_fractial_part(transfer_asset.amount.frac_part);
        return pb_transfer_asset;
      }

      model::TransferAsset PbCommandFactory::deserializeTransferAsset(
          const protocol::TransferAsset &pb_subtract_asset_quantity) {
        model::TransferAsset transfer_asset;
        transfer_asset.src_account_id =
            pb_subtract_asset_quantity.src_account_id();
        transfer_asset.dest_account_id =
            pb_subtract_asset_quantity.dest_account_id();
        transfer_asset.asset_id = pb_subtract_asset_quantity.asset_id();
        transfer_asset.description = pb_subtract_asset_quantity.description();
        transfer_asset.amount.int_part =
            pb_subtract_asset_quantity.amount().integer_part();
        transfer_asset.amount.frac_part =
            pb_subtract_asset_quantity.amount().fractial_part();
        return transfer_asset;
      }

      protocol::Command PbCommandFactory::serializeAbstractCommand(
          const model::Command &command) {
        PbCommandFactory commandFactory;
        auto cmd = protocol::Command();

        // -----|AddAssetQuantity|-----
        if (instanceof <model::AddAssetQuantity>(command)) {
          auto serialized = commandFactory.serializeAddAssetQuantity(
              static_cast<const model::AddAssetQuantity &>(command));
          cmd.set_allocated_add_asset_quantity(
              new protocol::AddAssetQuantity(serialized));
        }

        // -----|AddPeer|-----
        if (instanceof <model::AddPeer>(command)) {
          auto serialized = commandFactory.serializeAddPeer(
              static_cast<const model::AddPeer &>(command));
          cmd.set_allocated_add_peer(new protocol::AddPeer(serialized));
        }

        // -----|AddSignatory|-----
        if (instanceof <model::AddSignatory>(command)) {
          auto serialized = commandFactory.serializeAddSignatory(
              static_cast<const model::AddSignatory &>(command));
          cmd.set_allocated_add_signatory(
              new protocol::AddSignatory(serialized));
        }

        // -----|CreateAsset|-----
        if (instanceof<model::CreateAsset>(command)) {
          auto serialized = commandFactory.serializeCreateAsset(
                  static_cast<const model::CreateAsset &>(command));
          cmd.set_allocated_create_asset(new protocol::CreateAsset(serialized));
        }

        // -----|CreateAccount|-----
        if (instanceof <model::CreateAccount>(command)) {
          auto serialized = commandFactory.serializeCreateAccount(
              static_cast<const model::CreateAccount &>(command));
          cmd.set_allocated_create_account(
              new protocol::CreateAccount(serialized));
        }

        // -----|CreateDomain|-----
        if (instanceof <model::CreateDomain>(command)) {
          auto serialized = commandFactory.serializeCreateDomain(
              static_cast<const model::CreateDomain &>(command));
          cmd.set_allocated_create_domain(
              new protocol::CreateDomain(serialized));
        }

        // -----|RemoveSignatory|-----
        if (instanceof <model::RemoveSignatory>(command)) {
          auto serialized = commandFactory.serializeRemoveSignatory(
              static_cast<const model::RemoveSignatory &>(command));
          cmd.set_allocated_remove_sign(
              new protocol::RemoveSignatory(serialized));
        }

        // -----|SetAccountPermissions|-----
        if (instanceof <model::SetAccountPermissions>(command)) {
          auto serialized = commandFactory.serializeSetAccountPermissions(
              static_cast<const model::SetAccountPermissions &>(command));
          cmd.set_allocated_set_permission(
              new protocol::SetAccountPermissions(serialized));
        }

        // -----|SetAccountQuorum|-----
        if (instanceof <model::SetQuorum>(command)) {
          auto serialized = commandFactory.serializeSetQuorum(
              static_cast<const model::SetQuorum &>(command));
          cmd.set_allocated_set_quorum(
              new protocol::SetAccountQuorum(serialized));
        }

        // -----|TransferAsset|-----
        if (instanceof <model::TransferAsset>(command)) {
          auto serialized = commandFactory.serializeTransferAsset(
              static_cast<const model::TransferAsset &>(command));
          cmd.set_allocated_transfer_asset(
              new protocol::TransferAsset(serialized));
        }

        return cmd;
      }

      std::shared_ptr<model::Command>
      PbCommandFactory::deserializeAbstractCommand(
          const protocol::Command &command) {
        PbCommandFactory commandFactory;
        std::shared_ptr<model::Command> val;

        // -----|AddAssetQuantity|-----
        if (command.has_add_asset_quantity()) {
          auto pb_command = command.add_asset_quantity();
          auto cmd = commandFactory.deserializeAddAssetQuantity(pb_command);
          val = std::make_shared<model::AddAssetQuantity>(cmd);
        }

        // -----|AddPeer|-----
        if (command.has_add_peer()) {
          auto pb_command = command.add_peer();
          auto cmd = commandFactory.deserializeAddPeer(pb_command);
          val = std::make_shared<model::AddPeer>(cmd);
        }

        // -----|AddSignatory|-----
        if (command.has_add_signatory()) {
          auto pb_command = command.add_signatory();
          auto cmd = commandFactory.deserializeAddSignatory(pb_command);
          val = std::make_shared<model::AddSignatory>(cmd);
        }

        // -----|CreateAsset|-----
        if (command.has_create_asset()) {
          auto pb_command = command.create_asset();
          auto cmd = commandFactory.deserializeCreateAsset(pb_command);
          val = std::make_shared<model::CreateAsset>(cmd);
        }

        // -----|CreateAccount|-----
        if (command.has_create_account()) {
          auto pb_command = command.create_account();
          auto cmd = commandFactory.deserializeCreateAccount(pb_command);
          val = std::make_shared<model::CreateAccount>(cmd);
        }

        // -----|CreateDomain|-----
        if (command.has_create_domain()) {
          auto pb_command = command.create_domain();
          auto cmd = commandFactory.deserializeCreateDomain(pb_command);
          val = std::make_shared<model::CreateDomain>(cmd);
        }

        // -----|RemoveSignatory|-----
        if (command.has_remove_sign()) {
          auto pb_command = command.remove_sign();
          auto cmd = commandFactory.deserializeRemoveSignatory(pb_command);
          val = std::make_shared<model::RemoveSignatory>(cmd);
        }

        // -----|SetAccountPermissions|-----
        if (command.has_set_permission()) {
          auto pb_command = command.set_permission();
          auto cmd =
              commandFactory.deserializeSetAccountPermissions(pb_command);
          val = std::make_shared<model::SetAccountPermissions>(cmd);
        }

        // -----|SetAccountQuorum|-----
        if (command.has_set_quorum()) {
          auto pb_command = command.set_quorum();
          auto cmd = commandFactory.deserializeSetQuorum(pb_command);
          val = std::make_shared<model::SetQuorum>(cmd);
        }

        // -----|TransferAsset|-----
        if (command.has_transfer_asset()) {
          auto pb_command = command.transfer_asset();
          auto cmd = commandFactory.deserializeTransferAsset(pb_command);
          val = std::make_shared<model::TransferAsset>(cmd);
        }

        return val;
      }

    }  // namespace converters
  }    // namespace model
}  // namespace iroha
