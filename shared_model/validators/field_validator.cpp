/**
 * Copyright Soramitsu Co., Ltd. 2018 All Rights Reserved.
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

#include "validators/field_validator.hpp"
#include <boost/algorithm/string_regex.hpp>
#include <boost/format.hpp>
#include "permissions.hpp"
#include "cryptography/crypto_provider/crypto_verifier.hpp"

// TODO: 15.02.18 nickaleks Change structure to compositional IR-978

namespace shared_model {
  namespace validation {

    const std::string FieldValidator::account_name_pattern_ =
        R"#([a-z_0-9]{1,32})#";
    const std::string FieldValidator::asset_name_pattern_ =
        R"#([a-z_0-9]{1,32})#";
    const std::string FieldValidator::domain_pattern_ =
        R"#(([a-zA-Z]([a-zA-Z0-9\-]{0,61}[a-zA-Z0-9])?\.)*[a-zA-Z]([a-zA-Z0-9\-]{0,61}[a-zA-Z0-9])?)#";
    const std::string FieldValidator::ip_v4_pattern_ =
        R"#(^((([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\.){3})#"
        R"#(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])))#";
    const std::string FieldValidator::peer_address_pattern_ = "(("
        + ip_v4_pattern_ + ")|(" + domain_pattern_ + ")):"
        + R"#((6553[0-5]|655[0-2]\d|65[0-4]\d\d|6[0-4]\d{3}|[1-5]\d{4}|[1-9]\d{0,3}|0)$)#";
    const std::string FieldValidator::account_id_pattern_ =
        account_name_pattern_ + R"#(\@)#" + domain_pattern_;
    const std::string FieldValidator::asset_id_pattern_ =
        asset_name_pattern_ + R"#(\#)#" + domain_pattern_;
    const std::string FieldValidator::detail_key_pattern_ =
        R"([A-Za-z0-9_]{1,64})";
    const std::string FieldValidator::role_id_pattern_ = R"#([a-z_0-9]{1,32})#";

    const size_t FieldValidator::public_key_size = 32;
    const size_t FieldValidator::value_size = 4096;
    const size_t FieldValidator::description_size = 64;

    FieldValidator::FieldValidator(time_t future_gap)
        : account_name_regex_(account_name_pattern_),
          asset_name_regex_(asset_name_pattern_),
          domain_regex_(domain_pattern_),
          ip_v4_regex_(ip_v4_pattern_),
          peer_address_regex_(peer_address_pattern_),
          account_id_regex_(account_id_pattern_),
          asset_id_regex_(asset_id_pattern_),
          detail_key_regex_(detail_key_pattern_),
          role_id_regex_(role_id_pattern_),
          future_gap_(future_gap) {}

    void FieldValidator::validateAccountId(
        ReasonsGroupType &reason,
        const interface::types::AccountIdType &account_id) const {
      if (not std::regex_match(account_id, account_id_regex_)) {
        auto message =
            (boost::format("Wrongly formed account_id, passed value: '%s'. "
                           "Field should match regex '%s'")
             % account_id % account_id_pattern_)
                .str();
        reason.second.push_back(std::move(message));
      }
    }

    void FieldValidator::validateAssetId(
        ReasonsGroupType &reason,
        const interface::types::AssetIdType &asset_id) const {
      if (not std::regex_match(asset_id, asset_id_regex_)) {
        auto message = (boost::format("Wrongly formed asset_id, passed value: "
                                      "'%s'. Field should match regex '%s'")
                        % asset_id % asset_id_pattern_)
                           .str();
        reason.second.push_back(std::move(message));
      }
    }

    void FieldValidator::validatePeer(ReasonsGroupType &reason,
                                      const interface::Peer &peer) const {
      validatePeerAddress(reason, peer.address());
      validatePubkey(reason, peer.pubkey());
    }

    void FieldValidator::validateAmount(ReasonsGroupType &reason,
                                        const interface::Amount &amount) const {
      if (amount.intValue() <= 0) {
        auto message =
            (boost::format("Amount must be greater than 0, passed value: %d")
             % amount.intValue())
                .str();
        reason.second.push_back(message);
      }
    }

    void FieldValidator::validatePubkey(
        ReasonsGroupType &reason,
        const interface::types::PubkeyType &pubkey) const {
      if (pubkey.blob().size() != public_key_size) {
        auto message = (boost::format("Public key has wrong size, passed size: "
                                      "%d. Expected size: %d")
                        % pubkey.blob().size() % public_key_size)
                           .str();
        reason.second.push_back(std::move(message));
      }
    }

    void FieldValidator::validatePeerAddress(
        ReasonsGroupType &reason,
        const interface::types::AddressType &address) const {
      if (not std::regex_match(address, peer_address_regex_)) {
        auto message =
            (boost::format("Wrongly formed peer address, passed value: '%s'. "
                           "Field should have valid IPv4 format or be a valid "
                           "hostname following RFC1123 specification")
             % address)
                .str();
        reason.second.push_back(std::move(message));
      }
    }

    void FieldValidator::validateRoleId(
        ReasonsGroupType &reason,
        const interface::types::RoleIdType &role_id) const {
      if (not std::regex_match(role_id, role_id_regex_)) {
        auto message = (boost::format("Wrongly formed role_id, passed value: "
                                      "'%s'. Field should match regex '%s'")
                        % role_id % role_id_pattern_)
                           .str();
        reason.second.push_back(std::move(message));
      }
    }

    void FieldValidator::validateAccountName(
        ReasonsGroupType &reason,
        const interface::types::AccountNameType &account_name) const {
      if (not std::regex_match(account_name, account_name_regex_)) {
        auto message =
            (boost::format("Wrongly formed account_name, passed value: '%s'. "
                           "Field should match regex '%s'")
             % account_name % account_name_pattern_)
                .str();
        reason.second.push_back(std::move(message));
      }
    }

    void FieldValidator::validateDomainId(
        ReasonsGroupType &reason,
        const interface::types::DomainIdType &domain_id) const {
      if (not std::regex_match(domain_id, domain_regex_)) {
        auto message = (boost::format("Wrongly formed domain_id, passed value: "
                                      "'%s'. Field should match regex '%s'")
                        % domain_id % domain_pattern_)
                           .str();
        reason.second.push_back(std::move(message));
      }
    }

    void FieldValidator::validateAssetName(
        ReasonsGroupType &reason,
        const interface::types::AssetNameType &asset_name) const {
      if (not std::regex_match(asset_name, asset_name_regex_)) {
        auto message =
            (boost::format("Wrongly formed asset_name, passed value: '%s'. "
                           "Field should match regex '%s'")
             % asset_name % asset_name_pattern_)
                .str();
        reason.second.push_back(std::move(message));
      }
    }

    void FieldValidator::validateAccountDetailKey(
        ReasonsGroupType &reason,
        const interface::types::AccountDetailKeyType &key) const {
      if (not std::regex_match(key, detail_key_regex_)) {
        auto message = (boost::format("Wrongly formed key, passed value: '%s'. "
                                      "Field should match regex '%s'")
                        % key % detail_key_pattern_)
                           .str();
        reason.second.push_back(std::move(message));
      }
    }

    void FieldValidator::validateAccountDetailValue(
        ReasonsGroupType &reason,
        const interface::types::AccountDetailValueType &value) const {
      if (value.size() > value_size) {
        auto message =
            (boost::format("Detail value size should be less or equal '%d'")
             % value_size)
                .str();
        reason.second.push_back(std::move(message));
      }
    }

    void FieldValidator::validatePrecision(
        ReasonsGroupType &reason,
        const interface::types::PrecisionType &precision) const {
      // define precision constraints
    }

    void FieldValidator::validatePermission(
        ReasonsGroupType &reason,
        const interface::types::PermissionNameType &permission_name) const {
      if (iroha::model::all_perm_group.find(permission_name)
          == iroha::model::all_perm_group.end()) {
        reason.second.push_back("Provided permission does not exist");
      }
    }

    void FieldValidator::validatePermissions(
        ReasonsGroupType &reason,
        const interface::CreateRole::PermissionsType &permissions) const {
      if (permissions.empty()) {
        reason.second.push_back(
            "Permission set should contain at least one permission");
      }
      if (not std::includes(iroha::model::role_perm_group.begin(),
                            iroha::model::role_perm_group.end(),
                            permissions.begin(),
                            permissions.end())) {
        reason.second.push_back(
            "Provided permissions are not subset of the allowed permissions");
      }
    }

    void FieldValidator::validateQuorum(
        ReasonsGroupType &reason,
        const interface::types::QuorumType &quorum) const {
      if (quorum == 0 or quorum > 128) {
        reason.second.push_back("Quorum should be within range (0, 128]");
      }
    }

    void FieldValidator::validateCreatorAccountId(
        ReasonsGroupType &reason,
        const interface::types::AccountIdType &account_id) const {
      if (not std::regex_match(account_id, account_id_regex_)) {
        auto message =
            (boost::format("Wrongly formed creator_account_id, passed value: "
                           "'%s'. Field should match regex '%s'")
             % account_id % account_id_pattern_)
                .str();
        reason.second.push_back(std::move(message));
      }
    }

    void FieldValidator::validateCreatedTime(
        ReasonsGroupType &reason,
        const interface::types::TimestampType &timestamp) const {
      iroha::ts64_t now = iroha::time::now();

      if (now + future_gap_ < timestamp) {
        auto message = (boost::format("bad timestamp: sent from future, "
                                      "timestamp: %llu, now: %llu")
                        % timestamp % now)
                           .str();
        reason.second.push_back(std::move(message));
      }

      if (now > max_delay + timestamp) {
        auto message =
            (boost::format("bad timestamp: too old, timestamp: %llu, now: %llu")
             % timestamp % now)
                .str();
        reason.second.push_back(std::move(message));
      }
    }

    void FieldValidator::validateCounter(
        ReasonsGroupType &reason,
        const interface::types::CounterType &counter) const {
      if (counter <= 0) {
        auto message =
            (boost::format("Counter should be > 0, passed value: %d") % counter)
                .str();
        reason.second.push_back(message);
      }
    }

    void FieldValidator::validateSignatures(
        ReasonsGroupType &reason,
        const interface::SignatureSetType &signatures,
        const crypto::Blob &source) const {
      for (const auto &signature : signatures) {
        const auto &sign = signature->signedData();
        const auto &pkey = signature->publicKey();
        bool is_valid = true;

        if (sign.blob().size() != 64) {
          // TODO (@l4l) 03/02/18: IR-977 replace signature size with a const
          reason.second.push_back(
              (boost::format("Invalid signature: %s") % sign.hex()).str());
          is_valid = false;
        }

        if (pkey.blob().size() != 32) {
          // TODO (@l4l) 03/02/18: IR-977 replace public key size with a const
          reason.second.push_back(
              (boost::format("Invalid pubkey: %s") % pkey.hex()).str());
          is_valid = false;
        }

        if (is_valid
            && not shared_model::crypto::CryptoVerifier<>::verify(
                   sign, source, pkey)) {
          reason.second.push_back((boost::format("Wrong signature [%s;%s]")
                                   % sign.hex() % pkey.hex())
                                      .str());
        }
      }
    }

    void FieldValidator::validateDescription(
        shared_model::validation::ReasonsGroupType &reason,
        const shared_model::interface::types::DescriptionType &description)
        const {
      if (description.size() > description_size) {
        reason.second.push_back(
            (boost::format("Description size should be less or equal '%d'")
             % description_size)
                .str());
      }
    }

  }  // namespace validation
}  // namespace shared_model
