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

#ifndef IROHA_SHARED_MODEL_FIELD_VALIDATOR_HPP
#define IROHA_SHARED_MODEL_FIELD_VALIDATOR_HPP

#include <boost/format.hpp>
#include <regex>

#include "datetime/time.hpp"
#include "interfaces/commands/command.hpp"
#include "validator/address_validator.hpp"
#include "validators/answer.hpp"

namespace shared_model {
  namespace validation {

    /**
     * Class that validates fields of commands, concrete queries, transaction,
     * and query
     */
    class FieldValidator {
     public:
      FieldValidator()
          : account_id_(R"([a-z]{1,9}\@[a-z]{1,9})"),
            asset_id_(R"([a-z]{1,9}\#[a-z]{1,9})"),
            ip_address_(
                "((([0-1]?\\d\\d?)|((2[0-4]\\d)|(25[0-5]))).){3}(([0-1]?\\d\\d?"
                ")|((2[0-4]\\d)|(25[0-5])))"),
            name_(R"([a-z]{1,9})"),
            detail_key_(R"([A-Za-z0-9_]{1,})") {}

      void validateAccountId(
          ReasonsGroupType &reason,
          const interface::types::AccountIdType &account_id) const {
        if (not std::regex_match(account_id, account_id_)) {
          auto message =
              generateErrorMessage("Wrongly formed account_id",
                                   static_cast<std::string>(account_id));
          reason.second.push_back(std::move(message));
        }
      }

      void validateAssetId(
          ReasonsGroupType &reason,
          const interface::types::AssetIdType &asset_id) const {
        if (not std::regex_match(asset_id, asset_id_)) {
          auto message = generateErrorMessage(
              "Wrongly formed asset_id", static_cast<std::string>(asset_id));
          reason.second.push_back(std::move(message));
        }
      }

      void validateAmount(ReasonsGroupType &reason,
                          const interface::Amount &amount) const {
        /* put here any validations*/
      }

      void validatePubkey(ReasonsGroupType &reason,
                          const interface::types::PubkeyType &pubkey) const {
        if (pubkey.blob().size() != 32) {
          auto message = generateErrorMessage("Public key has wrong size",
                                              pubkey.hex());
          reason.second.push_back(std::move(message));
        }
      }

      void validatePeerAddress(
          ReasonsGroupType &reason,
          const interface::AddPeer::AddressType &address) const {
        if (not(iroha::validator::isValidIpV4(address)
                or iroha::validator::isValidHostname(address))) {
          auto message = generateErrorMessage(
              "Wrongly formed PeerAddress", static_cast<std::string>(address));
          reason.second.push_back(std::move(message));
        }
      }

      void validateRoleId(ReasonsGroupType &reason,
                          const interface::types::RoleIdType &role_id) const {
        if (not std::regex_match(role_id, name_)) {
          auto message = generateErrorMessage(
              "Wrongly formed role_id", static_cast<std::string>(role_id));
          reason.second.push_back(std::move(message));
        }
      }

      void validateAccountName(
          ReasonsGroupType &reason,
          const interface::types::AccountNameType &account_name) const {
        if (not std::regex_match(account_name, name_)) {
          auto message =
              generateErrorMessage("Wrongly formed account_name",
                                   static_cast<std::string>(account_name));
          reason.second.push_back(std::move(message));
        }
      }

      void validateDomainId(
          ReasonsGroupType &reason,
          const interface::types::DomainIdType &domain_id) const {
        if (not std::regex_match(domain_id, name_)) {
          auto message = generateErrorMessage(
              "Wrongly formed domain_id", static_cast<std::string>(domain_id));
          reason.second.push_back(std::move(message));
        }
      }

      void validateAssetName(
          ReasonsGroupType &reason,
          const interface::types::AssetNameType &asset_name) const {
        if (not std::regex_match(asset_name, name_)) {
          auto message =
              generateErrorMessage("Wrongly formed asset_name",
                                   static_cast<std::string>(asset_name));
          reason.second.push_back(std::move(message));
        }
      }

      void validateAccountDetailKey(
          ReasonsGroupType &reason,
          const interface::SetAccountDetail::AccountDetailKeyType &key) const {
        if (not std::regex_match(key, detail_key_)) {
          auto message = generateErrorMessage("Wrongly formed key",
                                              static_cast<std::string>(key));
          reason.second.push_back(std::move(message));
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

      void validateCreatorAccountId(
          ReasonsGroupType &reason,
          const interface::types::AccountIdType &account_id) const {
        if (not std::regex_match(account_id, account_id_)) {
          auto message =
              generateErrorMessage("Wrongly formed creator_account_id",
                                   static_cast<std::string>(account_id));
          reason.second.push_back(std::move(message));
        }
      }

      void validateCreatedTime(
          ReasonsGroupType &reason,
          const interface::types::TimestampType &timestamp) const {
        iroha::ts64_t now = iroha::time::now();
        // TODO 06/08/17 Muratov: make future gap for passing timestamp, like
        // with old timestamps IR-511 #goodfirstissue

        auto time_message =
            (boost::format("%llu, now: %llu") % timestamp % now).str();
        if (now < timestamp) {
          auto message = generateErrorMessage(
              "timestamp broken: sent from future", time_message);
          reason.second.push_back(std::move(message));
        }

        if (now - timestamp > MAX_DELAY) {
          auto message =
              generateErrorMessage("timestamp broken: too old ", time_message);
          reason.second.push_back(std::move(message));
        }
      }

      void validateCounter(ReasonsGroupType &reason,
                           const interface::types::CounterType &counter) const {
        if (counter == 0) {
          reason.second.push_back("Counter should be > 0");
        }
      }

     private:
      std::regex account_id_, asset_id_, ip_address_, name_, detail_key_;
      // max-delay between tx creation and validation
      static constexpr auto MAX_DELAY =
          std::chrono::hours(24) / std::chrono::milliseconds(1);

      std::string generateErrorMessage(const std::string &error,
                                       const std::string &field_value) const {
        return (boost::format("%s, passed value: %s") % error % field_value)
            .str();
      }
    };
  }  // namespace validation
}  // namespace shared_model

#endif  // IROHA_SHARED_MODEL_FIELD_VALIDATOR_HPP
