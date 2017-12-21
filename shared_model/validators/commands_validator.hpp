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

#ifndef IROHA_COMMANDS_VALIDATOR_HPP
#define IROHA_COMMANDS_VALIDATOR_HPP

#include <boost/format.hpp>
#include <boost/variant/static_visitor.hpp>
#include <regex>
#include "datetime/time.hpp"
#include "interfaces/common_objects/types.hpp"
#include "interfaces/transaction.hpp"
#include "utils/polymorphic_wrapper.hpp"
#include "validator/address_validator.hpp"
#include "validators/answer.hpp"

namespace shared_model {
  namespace validation {

    /**
     * Class that validates commands from transaction
     */
    class CommandsValidator {
     private:
      /**
       * Visitor used by commands validator to validate fields from tx commands
       */
      class CommandsValidatorVisitor
          : public boost::static_visitor<ReasonsGroupType> {
       public:
        ReasonsGroupType operator()(
            const detail::PolymorphicWrapper<interface::AddAssetQuantity> &aaq)
            const {
          ReasonsGroupType reason;
          reason.first = "AddAssetQuantity";

          validateAccountId(reason, aaq->accountId());
          validateAssetId(reason, aaq->assetId());
          validateAmount(reason, aaq->amount());

          return reason;
        }

        ReasonsGroupType operator()(
            const detail::PolymorphicWrapper<interface::AddPeer> &ap) const {
          ReasonsGroupType reason;
          reason.first = "AddPeer";

          validatePubkey(reason, ap->peerKey());
          validatePeerAddress(reason, ap->peerAddress());

          return reason;
        }

        ReasonsGroupType operator()(
            const detail::PolymorphicWrapper<interface::AddSignatory> &as)
            const {
          ReasonsGroupType reason;
          reason.first = "AddSignatory";

          validateAccountId(reason, as->accountId());
          validatePubkey(reason, as->pubkey());

          return reason;
        }

        ReasonsGroupType operator()(
            const detail::PolymorphicWrapper<interface::AppendRole> &ar) const {
          ReasonsGroupType reason;
          reason.first = "AppendRole";

          validateAccountId(reason, ar->accountId());
          validateRoleId(reason, ar->roleName());

          return reason;
        }

        ReasonsGroupType operator()(
            const detail::PolymorphicWrapper<interface::CreateAccount> &ca)
            const {
          ReasonsGroupType reason;
          reason.first = "CreateAccount";

          validatePubkey(reason, ca->pubkey());
          validateAccountName(reason, ca->accountName());

          return reason;
        }

        ReasonsGroupType operator()(
            const detail::PolymorphicWrapper<interface::CreateAsset> &ca)
            const {
          ReasonsGroupType reason;
          reason.first = "CreateAsset";

          validateAssetName(reason, ca->assetName());
          validateDomainId(reason, ca->domainId());
          validatePrecision(reason, ca->precision());

          return reason;
        }

        ReasonsGroupType operator()(
            const detail::PolymorphicWrapper<interface::CreateDomain> &cd)
            const {
          ReasonsGroupType reason;
          reason.first = "CreateDomain";

          validateDomainId(reason, cd->domainId());

          return reason;
        }

        ReasonsGroupType operator()(
            const detail::PolymorphicWrapper<interface::CreateRole> &cr) const {
          ReasonsGroupType reason;
          reason.first = "CreateRole";

          validateRoleId(reason, cr->roleName());
          validatePermissions(reason, cr->rolePermissions());

          return reason;
        }

        ReasonsGroupType operator()(
            const detail::PolymorphicWrapper<interface::DetachRole> &dr) const {
          ReasonsGroupType reason;
          reason.first = "DetachRole";

          validateAccountId(reason, dr->accountId());
          validateRoleId(reason, dr->roleName());

          return reason;
        }

        ReasonsGroupType operator()(
            const detail::PolymorphicWrapper<interface::GrantPermission> &gp)
            const {
          ReasonsGroupType reason;
          reason.first = "GrantPermission";

          validateAccountId(reason, gp->accountId());

          return reason;
        }

        ReasonsGroupType operator()(
            const detail::PolymorphicWrapper<interface::RemoveSignatory> &rs)
            const {
          ReasonsGroupType reason;
          reason.first = "RemoveSignatory";

          validateAccountId(reason, rs->accountId());
          validatePubkey(reason, rs->pubkey());

          return reason;
        }
        ReasonsGroupType operator()(
            const detail::PolymorphicWrapper<interface::RevokePermission> &rp)
            const {
          ReasonsGroupType reason;
          reason.first = "RevokePermission";

          validateAccountId(reason, rp->accountId());
          validatePermission(reason, rp->permissionName());

          return reason;
        }

        ReasonsGroupType operator()(
            const detail::PolymorphicWrapper<interface::SetAccountDetail> &sad)
            const {
          ReasonsGroupType reason;
          reason.first = "SetAccountDetail";

          validateAccountId(reason, sad->accountId());
          validateAccountDetailKey(reason, sad->key());

          return reason;
        }

        ReasonsGroupType operator()(
            const detail::PolymorphicWrapper<interface::SetQuorum> &sq) const {
          ReasonsGroupType reason;
          reason.first = "SetQuorum";

          validateAccountId(reason, sq->accountId());
          validateQuorum(reason, sq->newQuorum());

          return reason;
        }

        ReasonsGroupType operator()(
            const detail::PolymorphicWrapper<interface::SubtractAssetQuantity>
                &saq) const {
          ReasonsGroupType reason;
          reason.first = "SubtractAssetQuantity";

          validateAccountId(reason, saq->accountId());
          validateAssetId(reason, saq->assetId());
          validateAmount(reason, saq->amount());

          return reason;
        }

        ReasonsGroupType operator()(
            const detail::PolymorphicWrapper<interface::TransferAsset> &ta)
            const {
          ReasonsGroupType reason;
          reason.first = "TransferAsset";

          validateAccountId(reason, ta->srcAccountId());
          validateAccountId(reason, ta->destAccountId());
          validateAssetId(reason, ta->assetId());
          validateAmount(reason, ta->amount());

          return reason;
        }

       private:
        void validateAccountId(
            ReasonsGroupType &reason,
            const interface::types::AccountIdType &account_id) const {
          std::regex e(R"([a-z]{1,9}\@[a-z]{1,9})");
          if (not std::regex_match(account_id, e)) {
            reason.second.push_back("Wrongly formed account_id");
          }
        }

        void validateAssetId(
            ReasonsGroupType &reason,
            const interface::types::AssetIdType &asset_id) const {
          std::regex e(R"([a-z]{1,9}\#[a-z]{1,9})");
          if (not std::regex_match(asset_id, e)) {
            reason.second.push_back("Wrongly formed asset_id");
          }
        }

        void validateAmount(ReasonsGroupType &reason,
                            const interface::Amount &amount) const {
          // put here any validations
        }

        void validatePubkey(ReasonsGroupType &reason,
                            const interface::types::PubkeyType &pubkey) const {
          if (pubkey.blob().size() != 32) {
            reason.second.push_back("Public key has wrong size");
          }
        }

        void validatePeerAddress(
            ReasonsGroupType &reason,
            const interface::AddPeer::AddressType &address) const {
          if (not(iroha::validator::is_valid_ipv4(address)
                  or iroha::validator::is_valid_hostname(address))) {
            reason.second.push_back("Wrongly formed PeerAddress: " + address);
          }
        }

        void validateRoleId(ReasonsGroupType &reason,
                            const interface::types::RoleIdType &role_id) const {
          std::regex e(R"([a-z]{1,9})");
          if (not std::regex_match(role_id, e)) {
            reason.second.push_back("Wrongly formed role_id");
          }
        }

        void validateAccountName(
            ReasonsGroupType &reason,
            const interface::types::AccountNameType &account_name) const {
          std::regex e(R"([a-z]{1,9})");
          if (not std::regex_match(account_name, e)) {
            reason.second.push_back("Wrongly formed account_name");
          }
        }

        void validateDomainId(
            ReasonsGroupType &reason,
            const interface::types::DomainIdType &domain_id) const {
          std::regex e(R"([a-z]{1,9})");
          if (not std::regex_match(domain_id, e)) {
            reason.second.push_back("Wrongly formed domain_id");
          }
        }

        void validateAssetName(
            ReasonsGroupType &reason,
            const interface::types::AssetNameType &asset_name) const {
          std::regex e(R"([a-z]{1,9})");
          if (not std::regex_match(asset_name, e)) {
            reason.second.push_back("Wrongly formed asset_name");
          }
        }

        void validateAccountDetailKey(
            ReasonsGroupType &reason,
            const interface::SetAccountDetail::AccountDetailKeyType &key)
            const {
          std::regex e(R"([A-Za-z0-9_]{1,})");
          if (not std::regex_match(key, e)) {
            reason.second.push_back("Wrongly formed key");
          }
        }

        void validatePrecision(
            ReasonsGroupType &reason,
            const interface::types::PrecisionType &precision) const {
          // define precision constraints
        }

        void validatePermission(
            ReasonsGroupType &reason,
            const interface::types::PermissionNameType &permission_name) const {
          // define permission constraints
        }

        void validatePermissions(
            ReasonsGroupType &reason,
            const interface::CreateRole::PermissionsType &permissions) const {
          if (permissions.empty()) {
            reason.second.push_back(
                "Permission set should contain at least one permission");
          }
        }

        void validateQuorum(ReasonsGroupType &reason,
                            const interface::types::QuorumType &quorum) const {
          // define quorum constraints
        }
      };

     public:
      /**
       * Applies command validation on given tx
       * @param tx
       * @return Answer containing found error if any
       */
      Answer validate(
          detail::PolymorphicWrapper<interface::Transaction> tx) const {
        Answer answer;
        std::string tx_reason_name = "Transaction";
        ReasonsGroupType tx_reason(tx_reason_name, GroupedReasons());

        validateAmountOfCommands(tx_reason, tx->commands().size());
        validateCreatorAccountId(tx_reason, tx->creatorAccountId());
        validateCreatedTime(tx_reason, tx->createdTime());
        if (not tx_reason.second.empty()) {
          answer.addReason(std::move(tx_reason));
        }

        for (auto &command : tx->commands()) {
          auto reason =
              boost::apply_visitor(CommandsValidatorVisitor(), command->get());
          if (not reason.second.empty()) {
            answer.addReason(std::move(reason));
          }
        }

        return answer;
      }

     private:
      void validateAmountOfCommands(ReasonsGroupType &reason,
                                    size_t size) const {
        if (size == 0) {
          reason.second.push_back(
              "Transaction should contain at least one command");
        }
      }

      void validateCreatorAccountId(
          ReasonsGroupType &reason,
          const interface::types::AccountIdType &account_id) const {
        std::regex e(R"([a-z]{1,9}\@[a-z]{1,9})");
        if (not std::regex_match(account_id, e)) {
          reason.second.push_back("Wrongly formed creator_account_id");
        }
      }

      void validateCreatedTime(
          ReasonsGroupType &reason,
          const interface::types::TimestampType &timestamp) const {
        iroha::ts64_t now = iroha::time::now();
        // TODO 06/08/17 Muratov: make future gap for passing timestamp, like
        // with old timestamps IR-511 #goodfirstissue
        if (now < timestamp) {
          auto message =
              boost::format(
                  "timestamp broken: send from future (%llu, now %llu)")
              % timestamp % now;
          reason.second.push_back(message.str());
        }

        if (now - timestamp > MAX_DELAY) {
          auto message =
              boost::format("timestamp broken: too old (%llu, now %llu)")
              % timestamp % now;
          reason.second.push_back(message.str());
        }
      }

      // max-delay between tx creation and validation
      static constexpr auto MAX_DELAY =
          std::chrono::hours(24) / std::chrono::milliseconds(1);
      Answer answer_;
    };

  }  // namespace validation
}  // namespace shared_model

#endif  // IROHA_COMMANDS_VALIDATOR_HPP
