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
#include "interfaces/common_objects/pager.hpp"
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
          : account_id_pattern_(R"([a-z]{1,9}\@[a-z]{1,9})"),
            asset_id_pattern_(R"([a-z]{1,9}\#[a-z]{1,9})"),
            name_pattern_(R"([a-z]{1,9})"),
            detail_key_pattern_(R"([A-Za-z0-9_]{1,})") {}

      void validateAccountId(
          ReasonsGroupType &reason,
          const interface::types::AccountIdType &account_id) const {
        if (not std::regex_match(account_id, account_id_pattern_)) {
          auto message =
              (boost::format("Wrongly formed account_id, passed value: '%s'")
               % account_id)
                  .str();
          reason.second.push_back(std::move(message));
        }
      }

      void validateAssetId(
          ReasonsGroupType &reason,
          const interface::types::AssetIdType &asset_id) const {
        if (not std::regex_match(asset_id, asset_id_pattern_)) {
          auto message =
              (boost::format("Wrongly formed asset_id, passed value: '%s'")
               % asset_id)
                  .str();
          reason.second.push_back(std::move(message));
        }
      }

      void validateAssetsId(
          ReasonsGroupType &reason,
          const interface::types::AssetIdCollectionType &assets_id) const {
        if (std::any_of(assets_id.begin(),
                        assets_id.end(),
                        [this](const auto &asset_id) {
                          return not std::regex_match(asset_id,
                                                      asset_id_pattern_);
                        })) {
          reason.second.push_back("Wrongly formed asset_id in assets_id");
        }
      }

      void validateAmount(ReasonsGroupType &reason,
                          const interface::Amount &amount) const {
        if (amount.intValue() <= 0) {
          auto message =
              (boost::format("Amount must be greater than 0, passed value: %d")
               % amount.intValue())
                  .str();
          reason.second.push_back(message);
        }
      }

      void validatePager(ReasonsGroupType &reason,
                         const interface::Pager &pager) const {
        if (pager.transactionHash().size() > 0
            and pager.transactionHash().size() != iroha::hash256_t::size()) {
          auto message = (boost::format("Pager transaction hash size is "
                                        "invalid, passed value size: %d")
                          % pager.transactionHash().size())
                             .str();
          reason.second.push_back(message);
        }
        if (pager.limit() == 0
            or pager.limit() > interface::Pager::MaxLimit()) {
          auto message =
              (boost::format("Pager limit must be greater than 0 and less than "
                             "or equal to 100, passed value: %d")
               % pager.limit())
                  .str();
          reason.second.push_back(message);
        }
      }

      void validatePubkey(ReasonsGroupType &reason,
                          const interface::types::PubkeyType &pubkey) const {
        if (pubkey.blob().size() != key_size) {
          auto message =
              (boost::format("Public key has wrong size, passed size: %d")
               % pubkey.blob().size())
                  .str();
          reason.second.push_back(std::move(message));
        }
      }

      void validatePeerAddress(
          ReasonsGroupType &reason,
          const interface::AddPeer::AddressType &address) const {
        if (not(iroha::validator::isValidIpV4(address)
                or iroha::validator::isValidHostname(address))) {
          auto message =
              (boost::format("Wrongly formed peer address, passed value: '%s'")
               % address)
                  .str();
          reason.second.push_back(std::move(message));
        }
      }

      void validateRoleId(ReasonsGroupType &reason,
                          const interface::types::RoleIdType &role_id) const {
        if (not std::regex_match(role_id, name_pattern_)) {
          auto message =
              (boost::format("Wrongly formed role_id, passed value: '%s'")
               % role_id)
                  .str();
          reason.second.push_back(std::move(message));
        }
      }

      void validateAccountName(
          ReasonsGroupType &reason,
          const interface::types::AccountNameType &account_name) const {
        if (not std::regex_match(account_name, name_pattern_)) {
          auto message =
              (boost::format("Wrongly formed account_name, passed value: '%s'")
               % account_name)
                  .str();
          reason.second.push_back(std::move(message));
        }
      }

      void validateDomainId(
          ReasonsGroupType &reason,
          const interface::types::DomainIdType &domain_id) const {
        if (not std::regex_match(domain_id, name_pattern_)) {
          auto message =
              (boost::format("Wrongly formed domain_id, passed value: '%s'")
               % domain_id)
                  .str();
          reason.second.push_back(std::move(message));
        }
      }

      void validateAssetName(
          ReasonsGroupType &reason,
          const interface::types::AssetNameType &asset_name) const {
        if (not std::regex_match(asset_name, name_pattern_)) {
          auto message =
              (boost::format("Wrongly formed asset_name, passed value: '%s'")
               % asset_name)
                  .str();
          reason.second.push_back(std::move(message));
        }
      }

      void validateAccountDetailKey(
          ReasonsGroupType &reason,
          const interface::SetAccountDetail::AccountDetailKeyType &key) const {
        if (not std::regex_match(key, detail_key_pattern_)) {
          auto message =
              (boost::format("Wrongly formed key, passed value: '%s'") % key)
                  .str();
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
        if (not std::regex_match(account_id, account_id_pattern_)) {
          auto message =
              (boost::format(
                   "Wrongly formed creator_account_id, passed value: '%s'")
               % account_id)
                  .str();
          reason.second.push_back(std::move(message));
        }
      }

      void validateCreatedTime(
          ReasonsGroupType &reason,
          const interface::types::TimestampType &timestamp) const {
        iroha::ts64_t now = iroha::time::now();
        // TODO 06/08/17 Muratov: make future gap for passing timestamp, like
        // with old timestamps IR-511 #goodfirstissue

        if (now < timestamp) {
          auto message = (boost::format("bad timestamp: sent from future, "
                                        "timestamp: %llu, now: %llu")
                          % timestamp
                          % now)
                             .str();
          reason.second.push_back(std::move(message));
        }

        if (now - timestamp > max_delay) {
          auto message =
              (boost::format(
                   "bad timestamp: too old, timestamp: %llu, now: %llu")
               % timestamp
               % now)
                  .str();
          reason.second.push_back(std::move(message));
        }
      }

      void validateCounter(ReasonsGroupType &reason,
                           const interface::types::CounterType &counter) const {
        if (counter <= 0) {
          auto message =
              (boost::format("Counter should be > 0, passed value: %d")
               % counter)
                  .str();
          reason.second.push_back(message);
        }
      }

     private:
      std::regex account_id_pattern_;
      std::regex asset_id_pattern_;
      std::regex name_pattern_;
      std::regex detail_key_pattern_;
      // max-delay between tx creation and validation
      static constexpr auto max_delay =
          std::chrono::hours(24) / std::chrono::milliseconds(1);
      // size of key
      static constexpr auto key_size = 32;
    };
  }  // namespace validation
}  // namespace shared_model

#endif  // IROHA_SHARED_MODEL_FIELD_VALIDATOR_HPP
