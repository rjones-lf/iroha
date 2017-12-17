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

#include "block.pb.h"
#include "commands.pb.h"

#include "amount/amount.hpp"
#include "builders/protobuf/helpers.hpp"
#include "builders/protobuf/unsigned_proto.hpp"
#include "interfaces/common_objects/types.hpp"
#include "validators/default_validator.hpp"

namespace shared_model {
  namespace proto {
    template <int S = 0, typename SV = validation::DefaultValidator>
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
      using NextBuilder = TemplateTransactionBuilder<S | (1 << s)>;

      iroha::protocol::Transaction transaction_;

      template <int Sp>
      TemplateTransactionBuilder(const TemplateTransactionBuilder<Sp> &o,
                                 SV &&stateless_validator = SV())
          : transaction_(o.transaction_),
            stateless_validator_(std::forward<SV>(stateless_validator)) {}

      SV stateless_validator_;

     public:
      TemplateTransactionBuilder() = default;

      NextBuilder<CreatorAccountId> creatorAccountId(
          const interface::types::AccountIdType &account_id) {
        transaction_.mutable_payload()->set_creator_account_id(account_id);
        return *this;
      }

      NextBuilder<TxCounter> txCounter(
          interface::types::TxCounterType tx_counter) {
        transaction_.mutable_payload()->set_tx_counter(tx_counter);
        return *this;
      }

      NextBuilder<CreatedTime> createdTime(
          interface::types::TimestampType created_time) {
        transaction_.mutable_payload()->set_created_time(created_time);
        return *this;
      }

      NextBuilder<Command> addAssetQuantity(
          const interface::types::AccountIdType &account_id,
          const interface::types::AssetIdType &asset_id,
          const std::string &amount) {
        auto command = proto_command()->mutable_add_asset_quantity();
        command->set_account_id(account_id);
        command->set_asset_id(asset_id);
        addAmount(command->mutable_amount(), amount);
        return *this;
      }

      NextBuilder<Command> addPeer(
          const interface::types::AddressType &address,
          const interface::types::PubkeyType &peer_key) {
        auto command = proto_command()->mutable_add_peer();
        command->set_address(address);
        command->set_peer_key(peer_key.blob());
        return *this;
      }

      NextBuilder<Command> addSignatory(
          const interface::types::AddressType &account_id,
          const interface::types::PubkeyType &public_key) {
        auto command = proto_command()->mutable_add_signatory();
        command->set_account_id(account_id);
        command->set_public_key(public_key.blob());
        return *this;
      }

      NextBuilder<Command> removeSignatory(
          const interface::types::AddressType &account_id,
          const interface::types::PubkeyType &public_key) {
        auto command = proto_command()->mutable_remove_sign();
        command->set_account_id(account_id);
        command->set_public_key(public_key.blob());
        return *this;
      }

      NextBuilder<Command> createAsset(
          const interface::types::AssetNameType &asset_name,
          const interface::types::AddressType &domain_id,
          interface::types::PrecisionType precision) {
        auto command = proto_command()->mutable_create_asset();
        command->set_asset_name(asset_name);
        command->set_domain_id(domain_id);
        command->set_precision(precision);
        return *this;
      }

      NextBuilder<Command> createAccount(
          const interface::types::AccountNameType &account_name,
          const interface::types::AddressType &domain_id,
          const interface::types::PubkeyType &main_pubkey) {
        auto command = proto_command()->mutable_create_account();
        command->set_account_name(account_name);
        command->set_domain_id(domain_id);
        command->set_main_pubkey(main_pubkey.blob());
        return *this;
      }

      NextBuilder<Command> createDomain(
          const interface::types::AddressType &domain_id,
          const interface::types::RoleIdType &default_role) {
        auto command = proto_command()->mutable_create_domain();
        command->set_domain_id(domain_id);
        command->set_default_role(default_role);
        return *this;
      }

      NextBuilder<Command> setAccountQuorum(
          const interface::types::AddressType &account_id,
          interface::types::QuorumType quorum) {
        auto command = proto_command()->mutable_set_quorum();
        command->set_account_id(account_id);
        command->set_quorum(quorum);
        return *this;
      }

      NextBuilder<Command> transferAsset(
          const interface::types::AccountIdType &src_account_id,
          const interface::types::AccountIdType &dest_account_id,
          const interface::types::AssetIdType &asset_id,
          const std::string &description,
          const std::string &amount) {
        auto command = proto_command()->mutable_transfer_asset();
        command->set_src_account_id(src_account_id);
        command->set_dest_account_id(dest_account_id);
        command->set_asset_id(asset_id);
        command->set_description(description);
        addAmount(command->mutable_amount(), amount);
        return *this;
      }

      UnsignedWrapper<Transaction> build() {
        static_assert(S == (1 << TOTAL) - 1, "Required fields are not set");

        auto answer = stateless_validator_.validate(
            detail::make_polymorphic<Transaction>(transaction_));
        if (answer.hasErrors()) {
          throw std::invalid_argument(answer.reason());
        }
        return UnsignedWrapper<Transaction>(
            Transaction(iroha::protocol::Transaction(transaction_)));
      }

      static const int total = RequiredFields::TOTAL;

     private:
      iroha::protocol::Command *proto_command() {
        return transaction_.mutable_payload()->add_commands();
      }
    };

    using TransactionBuilder = TemplateTransactionBuilder<>;
  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_PROTO_TRANSACTION_BUILDER_HPP
