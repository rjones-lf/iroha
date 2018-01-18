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

#ifndef IROHA_PROTO_TRANSACTION_BUILDER_HPP
#define IROHA_PROTO_TRANSACTION_BUILDER_HPP

#include "backend/protobuf/transaction.hpp"

#include <boost/range/algorithm/for_each.hpp>

#include "block.pb.h"
#include "commands.pb.h"

#include "amount/amount.hpp"
#include "builders/protobuf/helpers.hpp"
#include "builders/protobuf/unsigned_proto.hpp"
#include "interfaces/common_objects/types.hpp"
#include "validators/default_validator.hpp"

namespace shared_model {
  namespace proto {

    template <int S = 0, typename SV = validation::DefaultTransactionValidator>
    class TemplateTransactionBuilder {
     private:
      template <int, typename>
      friend class TemplateTransactionBuilder;

      enum RequiredFields {
        Command,
        CreatorAccountId,
        TxCounter,
        CreatedTime,
        TOTAL
      };

      template <int s>
      using NextBuilder = TemplateTransactionBuilder<S | (1 << s), SV>;

      using ProtoTx = iroha::protocol::Transaction;
      using ProtoCommand = iroha::protocol::Command;

      template <int Sp>
      TemplateTransactionBuilder(const TemplateTransactionBuilder<Sp> &o)
          : transaction_(o.transaction_),
            stateless_validator_(o.stateless_validator_) {}

      /**
       * Make transformation on copied content
       * @tparam Transformation - callable type for changing the copy
       * @param f - transform function for proto object
       * @return new builder with updated state
       */
      template <int Fields, typename Transformation>
      auto transform(Transformation t) const {
        NextBuilder<Fields> copy = *this;
        t(copy.transaction_);
        return copy;
      }

      /**
       * Make add command transformation on copied object
       * @tparam Transformation - callable type for changing command
       * @param f - transform function for proto command
       * @return new builder with added command
       */
      template <typename Transformation>
      auto addCommand(Transformation t) const {
        NextBuilder<Command> copy = *this;
        t(copy.transaction_.mutable_payload()->add_commands());
        return copy;
      }

     public:
      TemplateTransactionBuilder(const SV &validator = SV())
          : stateless_validator_(validator) {}

      auto creatorAccountId(
          const interface::types::AccountIdType &account_id) const {
        return transform<CreatorAccountId>([&](auto &tx) {
          tx.mutable_payload()->set_creator_account_id(account_id);
        });
      }

      auto txCounter(interface::types::CounterType tx_counter) const {
        return transform<TxCounter>([&](auto &tx) {
          tx.mutable_payload()->set_tx_counter(tx_counter);
        });
      }

      auto createdTime(interface::types::TimestampType created_time) const {
        return transform<CreatedTime>([&](auto &tx) {
          tx.mutable_payload()->set_created_time(created_time);
        });
      }

      auto addAssetQuantity(const interface::types::AccountIdType &account_id,
                            const interface::types::AssetIdType &asset_id,
                            const std::string &amount) const {
        return addCommand([&](auto proto_command) {
          auto command = proto_command->mutable_add_asset_quantity();
          command->set_account_id(account_id);
          command->set_asset_id(asset_id);
          addAmount(command->mutable_amount(), amount);
        });
      }

      auto addPeer(const interface::types::AddressType &address,
                   const interface::types::PubkeyType &peer_key) const {
        return addCommand([&](auto proto_command) {
          auto command = proto_command->mutable_add_peer();
          command->set_address(address);
          command->set_peer_key(crypto::toBinaryString(peer_key));
        });
      }

      auto addSignatory(const interface::types::AccountIdType &account_id,
                        const interface::types::PubkeyType &public_key) const {
        return addCommand([&](auto proto_command) {
          auto command = proto_command->mutable_add_signatory();
          command->set_account_id(account_id);
          command->set_public_key(crypto::toBinaryString(public_key));
        });
      }

      auto removeSignatory(
          const interface::types::AccountIdType &account_id,
          const interface::types::PubkeyType &public_key) const {
        return addCommand([&](auto proto_command) {
          auto command = proto_command->mutable_remove_sign();
          command->set_account_id(account_id);
          command->set_public_key(crypto::toBinaryString(public_key));
        });
      }

      auto appendRole(const interface::types::AccountIdType &account_id,
                      const interface::types::RoleIdType &role_name) const {
        return addCommand([&](auto proto_command) {
          auto command = proto_command->mutable_append_role();
          command->set_account_id(account_id);
          command->set_role_name(role_name);
        });
      }

      auto createAsset(const interface::types::AssetNameType &asset_name,
                       const interface::types::DomainIdType &domain_id,
                       interface::types::PrecisionType precision) const {
        return addCommand([&](auto proto_command) {
          auto command = proto_command->mutable_create_asset();
          command->set_asset_name(asset_name);
          command->set_domain_id(domain_id);
          command->set_precision(precision);
        });
      }

      auto createAccount(
          const interface::types::AccountNameType &account_name,
          const interface::types::DomainIdType &domain_id,
          const interface::types::PubkeyType &main_pubkey) const {
        return addCommand([&](auto proto_command) {
          auto command = proto_command->mutable_create_account();
          command->set_account_name(account_name);
          command->set_domain_id(domain_id);
          command->set_main_pubkey(crypto::toBinaryString(main_pubkey));
        });
      }

      auto createDomain(
          const interface::types::DomainIdType &domain_id,
          const interface::types::RoleIdType &default_role) const {
        return addCommand([&](auto proto_command) {
          auto command = proto_command->mutable_create_domain();
          command->set_domain_id(domain_id);
          command->set_default_role(default_role);
        });
      }

      auto createRole(
          const interface::types::RoleIdType &role_name,
          std::initializer_list<interface::types::PermissionNameType>
              permissions) const {
        return createRole(role_name, permissions);
      }

      template <typename... Permission>
      auto createRole(const interface::types::RoleIdType &role_name,
                      const Permission &... permissions) const {
        return createRole(role_name, {permissions...});
      }

      template <typename Collection>
      auto createRole(const interface::types::RoleIdType &role_name,
                      const Collection &permissions) const {
        return addCommand([&](auto proto_command) {
          auto command = proto_command->mutable_create_role();
          command->set_role_name(role_name);
          boost::for_each(permissions, [&command](const auto &perm) {
            iroha::protocol::RolePermission p;
            iroha::protocol::RolePermission_Parse(perm, &p);
            command->add_permissions(p);
          });
        });
      }

      auto detachRole(const interface::types::AccountIdType &account_id,
                      const interface::types::RoleIdType &role_name) const {
        return addCommand([&](auto proto_command) {
          auto command = proto_command->mutable_detach_role();
          command->set_account_id(account_id);
          command->set_role_name(role_name);
        });
      }

      auto grantPermission(
          const interface::types::AccountIdType &account_id,
          const interface::types::PermissionNameType &permission) const {
        return addCommand([&](auto proto_command) {
          auto command = proto_command->mutable_grant_permission();
          command->set_account_id(account_id);
          iroha::protocol::GrantablePermission p;
          iroha::protocol::GrantablePermission_Parse(permission, &p);
          command->set_permission(p);
        });
      }

      auto revokePermission(
          const interface::types::AccountIdType &account_id,
          const interface::types::PermissionNameType &permission) const {
        return addCommand([&](auto proto_command) {
          auto command = proto_command->mutable_revoke_permission();
          command->set_account_id(account_id);
          iroha::protocol::GrantablePermission p;
          iroha::protocol::GrantablePermission_Parse(permission, &p);
          command->set_permission(p);
        });
      }

      auto setAccountDetail(
          const interface::types::AccountIdType &account_id,
          const interface::SetAccountDetail::AccountDetailKeyType &key,
          const interface::SetAccountDetail::AccountDetailValueType &value)
          const {
        return addCommand([&](auto proto_command) {
          auto command = proto_command->mutable_set_account_detail();
          command->set_account_id(account_id);
          command->set_key(key);
          command->set_value(value);
        });
      }

      auto setAccountQuorum(const interface::types::AddressType &account_id,
                            interface::types::QuorumType quorum) const {
        return addCommand([&](auto proto_command) {
          auto command = proto_command->mutable_set_quorum();
          command->set_account_id(account_id);
          command->set_quorum(quorum);
        });
      }

      auto subtractAssetQuantity(
          const interface::types::AccountIdType &account_id,
          const interface::types::AssetIdType &asset_id,
          const std::string &amount) const {
        return addCommand([&](auto proto_command) {
          auto command = proto_command->mutable_subtract_asset_quantity();
          command->set_account_id(account_id);
          command->set_asset_id(asset_id);
          addAmount(command->mutable_amount(), amount);
        });
      }

      auto transferAsset(
          const interface::types::AccountIdType &src_account_id,
          const interface::types::AccountIdType &dest_account_id,
          const interface::types::AssetIdType &asset_id,
          const interface::TransferAsset::MessageType &description,
          const std::string &amount) const {
        return addCommand([&](auto proto_command) {
          auto command = proto_command->mutable_transfer_asset();
          command->set_src_account_id(src_account_id);
          command->set_dest_account_id(dest_account_id);
          command->set_asset_id(asset_id);
          command->set_description(description);
          addAmount(command->mutable_amount(), amount);
        });
      }

      auto build() const {
        static_assert(S == (1 << TOTAL) - 1, "Required fields are not set");

        auto answer = stateless_validator_.validate(
            detail::makePolymorphic<Transaction>(transaction_));
        if (answer.hasErrors()) {
          throw std::invalid_argument(answer.reason());
        }
        return UnsignedWrapper<Transaction>(
            Transaction(iroha::protocol::Transaction(transaction_)));
      }

      static const int total = RequiredFields::TOTAL;

     private:
      ProtoTx transaction_;
      SV stateless_validator_;
    };

    using TransactionBuilder = TemplateTransactionBuilder<>;
  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_PROTO_TRANSACTION_BUILDER_HPP
