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

#include "model/converters/json_command_factory.hpp"

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

using namespace rapidjson;

namespace iroha {
  namespace model {
    namespace converters {
      template <>
      struct Convert<Amount> {
        template <typename T>
        auto operator()(T &&x) {
          auto des = makeFieldDeserializer(x);
          return nonstd::make_optional<Amount>()
              | des.Uint64(&Amount::int_part, "int_part")
              | des.Uint64(&Amount::frac_part, "frac_part");
        }
      };

      JsonCommandFactory::JsonCommandFactory() {
        serializers_ = {
            {typeid(AddAssetQuantity),
             &JsonCommandFactory::serializeAddAssetQuantity},
            {typeid(AddPeer), &JsonCommandFactory::serializeAddPeer},
            {typeid(AddSignatory), &JsonCommandFactory::serializeAddSignatory},
            {typeid(CreateAccount),
             &JsonCommandFactory::serializeCreateAccount},
            {typeid(CreateAsset), &JsonCommandFactory::serializeCreateAsset},
            {typeid(CreateDomain), &JsonCommandFactory::serializeCreateDomain},
            {typeid(RemoveSignatory),
             &JsonCommandFactory::serializeRemoveSignatory},
            {typeid(SetAccountPermissions),
             &JsonCommandFactory::serializeSetAccountPermissions},
            {typeid(SetQuorum), &JsonCommandFactory::serializeSetQuorum},
            {typeid(TransferAsset),
             &JsonCommandFactory::serializeTransferAsset}};

        deserializers_ = {
            {"AddAssetQuantity",
             &JsonCommandFactory::deserializeAddAssetQuantity},
            {"AddPeer", &JsonCommandFactory::deserializeAddPeer},
            {"AddSignatory", &JsonCommandFactory::deserializeAddSignatory},
            {"CreateAccount", &JsonCommandFactory::deserializeCreateAccount},
            {"CreateAsset", &JsonCommandFactory::deserializeCreateAsset},
            {"CreateDomain", &JsonCommandFactory::deserializeCreateDomain},
            {"RemoveSignatory",
             &JsonCommandFactory::deserializeRemoveSignatory},
            {"SetAccountPermissions",
             &JsonCommandFactory::deserializeSetAccountPermissions},
            {"SetQuorum", &JsonCommandFactory::deserializeSetQuorum},
            {"TransferAsset", &JsonCommandFactory::deserializeTransferAsset}};
      }

      // AddAssetQuantity
      Document JsonCommandFactory::serializeAddAssetQuantity(
          std::shared_ptr<Command> command) {
        auto add_asset_quantity =
            static_cast<AddAssetQuantity *>(command.get());

        Document document;
        auto &allocator = document.GetAllocator();

        document.SetObject();
        document.AddMember("command_type", "AddAssetQuantity", allocator);
        document.AddMember("account_id", add_asset_quantity->account_id,
                           allocator);
        document.AddMember("asset_id", add_asset_quantity->asset_id, allocator);

        Value amount;
        amount.SetObject();
        amount.AddMember("int_part", add_asset_quantity->amount.int_part,
                         allocator);
        amount.AddMember("frac_part", add_asset_quantity->amount.frac_part,
                         allocator);

        document.AddMember("amount", amount, allocator);

        return document;
      }

      optional_ptr<Command> JsonCommandFactory::deserializeAddAssetQuantity(
          const Value &document) {
        auto des = makeFieldDeserializer(document);
        return make_optional_ptr<AddAssetQuantity>()
            | des.String(&AddAssetQuantity::account_id, "account_id")
            | des.String(&AddAssetQuantity::asset_id, "asset_id")
            | des.Object(&AddAssetQuantity::amount, "amount")
            | toCommand;
      }

      // AddPeer
      Document JsonCommandFactory::serializeAddPeer(
          std::shared_ptr<Command> command) {
        auto add_peer = static_cast<AddPeer *>(command.get());

        Document document;
        auto &allocator = document.GetAllocator();

        document.SetObject();
        document.AddMember("command_type", "AddPeer", allocator);
        document.AddMember("address", add_peer->address, allocator);
        document.AddMember("peer_key", add_peer->peer_key.to_hexstring(),
                           allocator);

        return document;
      }

      optional_ptr<Command> JsonCommandFactory::deserializeAddPeer(
          const Value &document) {
        auto des = makeFieldDeserializer(document);
        return make_optional_ptr<AddPeer>()
            | des.String(&AddPeer::peer_key, "peer_key")
            | des.String(&AddPeer::address, "address")
            | toCommand;
      }

      // AddSignatory
      Document JsonCommandFactory::serializeAddSignatory(
          std::shared_ptr<Command> command) {
        auto add_signatory = static_cast<AddSignatory *>(command.get());

        Document document;
        auto &allocator = document.GetAllocator();

        document.SetObject();
        document.AddMember("command_type", "AddSignatory", allocator);
        document.AddMember("account_id", add_signatory->account_id, allocator);
        document.AddMember("pubkey", add_signatory->pubkey.to_hexstring(),
                           allocator);

        return document;
      }

      optional_ptr<Command> JsonCommandFactory::deserializeAddSignatory(
          const Value &document) {
        auto des = makeFieldDeserializer(document);
        return make_optional_ptr<AddSignatory>()
            | des.String(&AddSignatory::account_id, "account_id")
            | des.String(&AddSignatory::pubkey, "pubkey")
            | toCommand;
      }

      // CreateAccount
      Document JsonCommandFactory::serializeCreateAccount(
          std::shared_ptr<Command> command) {
        auto create_account = static_cast<CreateAccount *>(command.get());

        Document document;
        auto &allocator = document.GetAllocator();

        document.SetObject();
        document.AddMember("command_type", "CreateAccount", allocator);
        document.AddMember("account_name", create_account->account_name,
                           allocator);
        document.AddMember("domain_id", create_account->domain_id, allocator);
        document.AddMember("pubkey", create_account->pubkey.to_hexstring(),
                           allocator);

        return document;
      }

      optional_ptr<Command> JsonCommandFactory::deserializeCreateAccount(
          const Value &document) {
        auto des = makeFieldDeserializer(document);
        return make_optional_ptr<CreateAccount>()
            | des.String(&CreateAccount::account_name, "account_name")
            | des.String(&CreateAccount::domain_id, "domain_id")
            | des.String(&CreateAccount::pubkey, "pubkey")
            | toCommand;
      }

      // CreateAsset
      Document JsonCommandFactory::serializeCreateAsset(
          std::shared_ptr<Command> command) {
        auto create_asset = static_cast<CreateAsset *>(command.get());

        Document document;
        auto &allocator = document.GetAllocator();

        document.SetObject();
        document.AddMember("command_type", "CreateAsset", allocator);
        document.AddMember("asset_name", create_asset->asset_name, allocator);
        document.AddMember("domain_id", create_asset->domain_id, allocator);
        document.AddMember("precision", create_asset->precision, allocator);

        return document;
      }

      optional_ptr<Command> JsonCommandFactory::deserializeCreateAsset(
          const Value &document) {
        auto des = makeFieldDeserializer(document);
        return make_optional_ptr<CreateAsset>()
            | des.String(&CreateAsset::asset_name, "asset_name")
            | des.String(&CreateAsset::domain_id, "domain_id")
            | des.Uint(&CreateAsset::precision, "precision")
            | toCommand;
      }

      // CreateDomain
      Document JsonCommandFactory::serializeCreateDomain(
          std::shared_ptr<Command> command) {
        auto create_domain = static_cast<CreateDomain *>(command.get());

        Document document;
        auto &allocator = document.GetAllocator();

        document.SetObject();
        document.AddMember("command_type", "CreateDomain", allocator);
        document.AddMember("domain_name", create_domain->domain_name,
                           allocator);

        return document;
      }

      optional_ptr<Command> JsonCommandFactory::deserializeCreateDomain(
          const Value &document) {
        auto des = makeFieldDeserializer(document);
        return make_optional_ptr<CreateDomain>()
            | des.String(&CreateDomain::domain_name, "domain_name")
            | toCommand;
      }

      // RemoveSignatory
      Document JsonCommandFactory::serializeRemoveSignatory(
          std::shared_ptr<Command> command) {
        auto remove_signatory = static_cast<RemoveSignatory *>(command.get());

        Document document;
        auto &allocator = document.GetAllocator();

        document.SetObject();
        document.AddMember("command_type", "RemoveSignatory", allocator);
        document.AddMember("account_id", remove_signatory->account_id,
                           allocator);
        document.AddMember("pubkey", remove_signatory->pubkey.to_hexstring(),
                           allocator);

        return document;
      }

      optional_ptr<Command> JsonCommandFactory::deserializeRemoveSignatory(
          const Value &document) {
        auto des = makeFieldDeserializer(document);
        return make_optional_ptr<RemoveSignatory>()
            | des.String(&RemoveSignatory::account_id, "account_id")
            | des.String(&RemoveSignatory::pubkey, "pubkey")
            | toCommand;
      }

      // SetAccountPermissions
      Document JsonCommandFactory::serializeSetAccountPermissions(
          std::shared_ptr<Command> command) {
        auto set_account_permissions =
            static_cast<SetAccountPermissions *>(command.get());

        Document document;
        auto &allocator = document.GetAllocator();

        document.SetObject();
        document.AddMember("command_type", "SetAccountPermissions", allocator);
        document.AddMember("account_id", set_account_permissions->account_id,
                           allocator);

        Value new_permissions;
        new_permissions.SetObject();
        new_permissions.AddMember(
            "add_signatory",
            set_account_permissions->new_permissions.add_signatory, allocator);
        new_permissions.AddMember(
            "can_transfer",
            set_account_permissions->new_permissions.can_transfer, allocator);
        new_permissions.AddMember(
            "create_accounts",
            set_account_permissions->new_permissions.create_accounts,
            allocator);
        new_permissions.AddMember(
            "create_assets",
            set_account_permissions->new_permissions.create_assets, allocator);
        new_permissions.AddMember(
            "create_domains",
            set_account_permissions->new_permissions.create_domains, allocator);
        new_permissions.AddMember(
            "issue_assets",
            set_account_permissions->new_permissions.issue_assets, allocator);
        new_permissions.AddMember(
            "read_all_accounts",
            set_account_permissions->new_permissions.read_all_accounts,
            allocator);
        new_permissions.AddMember(
            "remove_signatory",
            set_account_permissions->new_permissions.remove_signatory,
            allocator);
        new_permissions.AddMember(
            "set_permissions",
            set_account_permissions->new_permissions.set_permissions,
            allocator);
        new_permissions.AddMember(
            "set_quorum", set_account_permissions->new_permissions.set_quorum,
            allocator);

        document.AddMember("new_permissions", new_permissions, allocator);

        return document;
      }

      optional_ptr<Command>
      JsonCommandFactory::deserializeSetAccountPermissions(
          const Value &document) {
        auto des = makeFieldDeserializer(document);
        return make_optional_ptr<SetAccountPermissions>()
            | des.String(&SetAccountPermissions::account_id, "account_id")
            | des.Object(&SetAccountPermissions::new_permissions,
                          "new_permissions", [](auto permissions) {
                  auto des = makeFieldDeserializer(permissions);
                  return nonstd::make_optional<Account::Permissions>()
                      | des.Bool(&Account::Permissions::add_signatory,
                                 "add_signatory")
                      | des.Bool(&Account::Permissions::can_transfer,
                                 "can_transfer")
                      | des.Bool(&Account::Permissions::create_accounts,
                                 "create_accounts")
                      | des.Bool(&Account::Permissions::create_assets,
                                 "create_assets")
                      | des.Bool(&Account::Permissions::create_domains,
                                 "create_domains")
                      | des.Bool(&Account::Permissions::issue_assets,
                                 "issue_assets")
                      | des.Bool(&Account::Permissions::read_all_accounts,
                                 "read_all_accounts")
                      | des.Bool(&Account::Permissions::remove_signatory,
                                 "remove_signatory")
                      | des.Bool(&Account::Permissions::set_permissions,
                                 "set_permissions")
                      | des.Bool(&Account::Permissions::set_quorum,
                                 "set_quorum");
                })
            | toCommand;
      }

      // SetAccountQuorum
      Document JsonCommandFactory::serializeSetQuorum(
          std::shared_ptr<Command> command) {
        auto set_quorum = static_cast<SetQuorum *>(command.get());

        Document document;
        auto &allocator = document.GetAllocator();

        document.SetObject();
        document.AddMember("command_type", "SetQuorum", allocator);
        document.AddMember("account_id", set_quorum->account_id, allocator);
        document.AddMember("new_quorum", set_quorum->new_quorum, allocator);

        return document;
      }

      optional_ptr<Command> JsonCommandFactory::deserializeSetQuorum(
          const Value &document) {
        auto des = makeFieldDeserializer(document);
        return make_optional_ptr<SetQuorum>()
            | des.String(&SetQuorum::account_id, "account_id")
            | des.Uint(&SetQuorum::new_quorum, "new_quorum")
            | toCommand;
      }

      // TransferAsset
      Document JsonCommandFactory::serializeTransferAsset(
          std::shared_ptr<Command> command) {
        auto transfer_asset = static_cast<TransferAsset *>(command.get());

        Document document;
        auto &allocator = document.GetAllocator();

        document.SetObject();
        document.AddMember("command_type", "TransferAsset", allocator);
        document.AddMember("src_account_id", transfer_asset->src_account_id,
                           allocator);
        document.AddMember("dest_account_id", transfer_asset->dest_account_id,
                           allocator);
        document.AddMember("asset_id", transfer_asset->asset_id, allocator);
        document.AddMember("description", transfer_asset->description,
                           allocator);

        Value amount;
        amount.SetObject();
        amount.AddMember("int_part", transfer_asset->amount.int_part,
                         allocator);
        amount.AddMember("frac_part", transfer_asset->amount.frac_part,
                         allocator);

        document.AddMember("amount", amount, allocator);

        return document;
      }

      optional_ptr<Command> JsonCommandFactory::deserializeTransferAsset(
          const Value &document) {
        auto des = makeFieldDeserializer(document);
        return make_optional_ptr<TransferAsset>()
            | des.String(&TransferAsset::src_account_id, "src_account_id")
            | des.String(&TransferAsset::dest_account_id, "dest_account_id")
            | des.String(&TransferAsset::asset_id, "asset_id")
            | des.String(&TransferAsset::description, "description")
            | des.Object(&TransferAsset::amount, "amount")
            | toCommand;
      }

      // Abstract
      Document JsonCommandFactory::serializeAbstractCommand(
          std::shared_ptr<Command> command) {
        return makeMethodInvoke(*this,
                                command)(serializers_.at(typeid(*command)));
      }

      optional_ptr<model::Command>
      JsonCommandFactory::deserializeAbstractCommand(const Value &document) {
        return makeFieldDeserializer(document).String("command_type")
            | makeOptionalGet(deserializers_)
            | makeMethodInvoke(*this, document);
      }
    }  // namespace converters
  }    // namespace model
}  // namespace iroha
