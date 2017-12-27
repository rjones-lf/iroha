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

#ifndef IROHA_SHARED_MODEL_TRANSACTION_VALIDATOR_HPP
#define IROHA_SHARED_MODEL_TRANSACTION_VALIDATOR_HPP

#include <boost/variant/static_visitor.hpp>

#include "interfaces/transaction.hpp"
#include "validators/answer.hpp"

namespace shared_model {
  namespace validation {

    /**
     * Visitor used by transaction validator to validate each command
     * @tparam FieldValidator - field validator type
     */
    template <typename FieldValidator>
    class CommandValidatorVisitor
        : public boost::static_visitor<ReasonsGroupType> {
     public:
      CommandValidatorVisitor(
          const FieldValidator &validator = FieldValidator())
          : validator_(validator) {}

      ReasonsGroupType operator()(
          const detail::PolymorphicWrapper<interface::AddAssetQuantity> &aaq)
          const {
        ReasonsGroupType reason;
        reason.first = "AddAssetQuantity";

        validator_.validateAccountId(reason, aaq->accountId());
        validator_.validateAssetId(reason, aaq->assetId());
        validator_.validateAmount(reason, aaq->amount());

        return reason;
      }

      ReasonsGroupType operator()(
          const detail::PolymorphicWrapper<interface::AddPeer> &ap) const {
        ReasonsGroupType reason;
        reason.first = "AddPeer";

        validator_.validatePubkey(reason, ap->peerKey());
        validator_.validatePeerAddress(reason, ap->peerAddress());

        return reason;
      }

      ReasonsGroupType operator()(
          const detail::PolymorphicWrapper<interface::AddSignatory> &as) const {
        ReasonsGroupType reason;
        reason.first = "AddSignatory";

        validator_.validateAccountId(reason, as->accountId());
        validator_.validatePubkey(reason, as->pubkey());

        return reason;
      }

      ReasonsGroupType operator()(
          const detail::PolymorphicWrapper<interface::AppendRole> &ar) const {
        ReasonsGroupType reason;
        reason.first = "AppendRole";

        validator_.validateAccountId(reason, ar->accountId());
        validator_.validateRoleId(reason, ar->roleName());

        return reason;
      }

      ReasonsGroupType operator()(
          const detail::PolymorphicWrapper<interface::CreateAccount> &ca)
          const {
        ReasonsGroupType reason;
        reason.first = "CreateAccount";

        validator_.validatePubkey(reason, ca->pubkey());
        validator_.validateAccountName(reason, ca->accountName());

        return reason;
      }

      ReasonsGroupType operator()(
          const detail::PolymorphicWrapper<interface::CreateAsset> &ca) const {
        ReasonsGroupType reason;
        reason.first = "CreateAsset";

        validator_.validateAssetName(reason, ca->assetName());
        validator_.validateDomainId(reason, ca->domainId());
        validator_.validatePrecision(reason, ca->precision());

        return reason;
      }

      ReasonsGroupType operator()(
          const detail::PolymorphicWrapper<interface::CreateDomain> &cd) const {
        ReasonsGroupType reason;
        reason.first = "CreateDomain";

        validator_.validateDomainId(reason, cd->domainId());

        return reason;
      }

      ReasonsGroupType operator()(
          const detail::PolymorphicWrapper<interface::CreateRole> &cr) const {
        ReasonsGroupType reason;
        reason.first = "CreateRole";

        validator_.validateRoleId(reason, cr->roleName());
        validator_.validatePermissions(reason, cr->rolePermissions());

        return reason;
      }

      ReasonsGroupType operator()(
          const detail::PolymorphicWrapper<interface::DetachRole> &dr) const {
        ReasonsGroupType reason;
        reason.first = "DetachRole";

        validator_.validateAccountId(reason, dr->accountId());
        validator_.validateRoleId(reason, dr->roleName());

        return reason;
      }

      ReasonsGroupType operator()(
          const detail::PolymorphicWrapper<interface::GrantPermission> &gp)
          const {
        ReasonsGroupType reason;
        reason.first = "GrantPermission";

        validator_.validateAccountId(reason, gp->accountId());

        return reason;
      }

      ReasonsGroupType operator()(
          const detail::PolymorphicWrapper<interface::RemoveSignatory> &rs)
          const {
        ReasonsGroupType reason;
        reason.first = "RemoveSignatory";

        validator_.validateAccountId(reason, rs->accountId());
        validator_.validatePubkey(reason, rs->pubkey());

        return reason;
      }
      ReasonsGroupType operator()(
          const detail::PolymorphicWrapper<interface::RevokePermission> &rp)
          const {
        ReasonsGroupType reason;
        reason.first = "RevokePermission";

        validator_.validateAccountId(reason, rp->accountId());
        validator_.validatePermission(reason, rp->permissionName());

        return reason;
      }

      ReasonsGroupType operator()(
          const detail::PolymorphicWrapper<interface::SetAccountDetail> &sad)
          const {
        ReasonsGroupType reason;
        reason.first = "SetAccountDetail";

        validator_.validateAccountId(reason, sad->accountId());
        validator_.validateAccountDetailKey(reason, sad->key());

        return reason;
      }

      ReasonsGroupType operator()(
          const detail::PolymorphicWrapper<interface::SetQuorum> &sq) const {
        ReasonsGroupType reason;
        reason.first = "SetQuorum";

        validator_.validateAccountId(reason, sq->accountId());
        validator_.validateQuorum(reason, sq->newQuorum());

        return reason;
      }

      ReasonsGroupType operator()(
          const detail::PolymorphicWrapper<interface::SubtractAssetQuantity>
              &saq) const {
        ReasonsGroupType reason;
        reason.first = "SubtractAssetQuantity";

        validator_.validateAccountId(reason, saq->accountId());
        validator_.validateAssetId(reason, saq->assetId());
        validator_.validateAmount(reason, saq->amount());

        return reason;
      }

      ReasonsGroupType operator()(
          const detail::PolymorphicWrapper<interface::TransferAsset> &ta)
          const {
        ReasonsGroupType reason;
        reason.first = "TransferAsset";

        validator_.validateAccountId(reason, ta->srcAccountId());
        validator_.validateAccountId(reason, ta->destAccountId());
        validator_.validateAssetId(reason, ta->assetId());
        validator_.validateAmount(reason, ta->amount());

        return reason;
      }

     private:
      FieldValidator validator_;
    };

    /**
     * Class that validates commands from transaction
     * @tparam FieldValidator
     * @tparam CommandValidator
     */
    template <typename FieldValidator, typename CommandValidator>
    class TransactionValidator {
     public:
      TransactionValidator(
          const FieldValidator &field_validator = FieldValidator(),
          const CommandValidator &command_validator = CommandValidator())
          : field_validator_(field_validator),
            command_validator_(command_validator) {}

      /**
       * Applies validation to given transaction
       * @param tx - transaction to validate
       * @return Answer containing found error if any
       */
      Answer validate(
          detail::PolymorphicWrapper<interface::Transaction> tx) const {
        Answer answer;
        std::string tx_reason_name = "Transaction";
        ReasonsGroupType tx_reason(tx_reason_name, GroupedReasons());

        if (tx->commands().empty()) {
          tx_reason.second.push_back(
              "Transaction should contain at least one command");
        }

        field_validator_.validateCreatorAccountId(tx_reason,
                                                  tx->creatorAccountId());
        field_validator_.validateCreatedTime(tx_reason, tx->createdTime());

        if (not tx_reason.second.empty()) {
          answer.addReason(std::move(tx_reason));
        }

        for (const auto &command : tx->commands()) {
          auto reason =
              boost::apply_visitor(command_validator_, command->get());
          if (not reason.second.empty()) {
            answer.addReason(std::move(reason));
          }
        }

        return answer;
      }

     private:
      Answer answer_;
      FieldValidator field_validator_;
      CommandValidator command_validator_;
    };

  }  // namespace validation
}  // namespace shared_model

#endif  // IROHA_SHARED_MODEL_TRANSACTION_VALIDATOR_HPP
