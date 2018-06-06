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

#include <regex>

#include "datetime/time.hpp"
#include "interfaces/base/signable.hpp"
#include "interfaces/commands/command.hpp"
#include "interfaces/transaction.hpp"
#include "interfaces/queries/query_payload_meta.hpp"
#include "validators/answer.hpp"

namespace shared_model {
  namespace validation {

    /**
     * Class that validates fields of commands, concrete queries, transaction,
     * and query
     */
    class FieldValidator {
     public:
      FieldValidator(time_t future_gap = default_future_gap);

      void validateAccountId(
          ReasonsGroupType &reason,
          const interface::types::AccountIdType &account_id) const;

      void validateAssetId(ReasonsGroupType &reason,
                           const interface::types::AssetIdType &asset_id) const;

      void validatePeer(ReasonsGroupType &reason,
                        const interface::Peer &peer) const;

      void validateAmount(ReasonsGroupType &reason,
                          const interface::Amount &amount) const;

      void validatePubkey(ReasonsGroupType &reason,
                          const interface::types::PubkeyType &pubkey) const;

      void validatePeerAddress(
          ReasonsGroupType &reason,
          const interface::types::AddressType &address) const;

      void validateRoleId(ReasonsGroupType &reason,
                          const interface::types::RoleIdType &role_id) const;

      void validateAccountName(
          ReasonsGroupType &reason,
          const interface::types::AccountNameType &account_name) const;

      // clang-format off
      /**
       * Check if the given string `domain_id` is in valid domain syntax defined in
       * the RFC 1035 and 1123. Return the result of the validation.
       *
       * The domain syntax in RFC 1035 is given below:
       *
       *   <domain>      ::= <subdomain> | ” ”
       *   <subdomain>   ::= <label> | <subdomain> “.” <label>
       *   <label>       ::= <letter> [ [ <ldh-str> ] <let-dig> ]
       *   <ldh-str>     ::= <let-dig-hyp> | <let-dig-hyp> <ldh-str>
       *   <let-dig-hyp> ::= <let-dig> | “-”
       *   <let-dig>     ::= <letter> | <digit>
       *   <letter>      ::= any one of the 52 alphabetic characters A through Z in
       *                     upper case and a through z in lower case
       *   <digit>       ::= any one of the ten digits 0 through 9
       *
       * And the subsequent RFC 1123 disallows the root white space.
       *
       * If the validation is not successful reason is updated with corresponding message
       */
      // clang-format on
      void validateDomainId(
          ReasonsGroupType &reason,
          const interface::types::DomainIdType &domain_id) const;

      void validateAssetName(
          ReasonsGroupType &reason,
          const interface::types::AssetNameType &asset_name) const;

      void validateAccountDetailKey(
          ReasonsGroupType &reason,
          const interface::types::AccountDetailKeyType &key) const;

      void validateAccountDetailValue(
          ReasonsGroupType &reason,
          const interface::types::AccountDetailValueType &value) const;

      void validatePrecision(
          ReasonsGroupType &reason,
          const interface::types::PrecisionType &precision) const;

      void validatePermission(
          ReasonsGroupType &reason,
          const interface::types::PermissionNameType &permission_name) const;

      void validatePermissions(
          ReasonsGroupType &reason,
          const interface::CreateRole::PermissionsType &permissions) const;

      void validateQuorum(ReasonsGroupType &reason,
                          const interface::types::QuorumType &quorum) const;

      void validateCreatorAccountId(
          ReasonsGroupType &reason,
          const interface::types::AccountIdType &account_id) const;

      void validateCreatedTime(
          ReasonsGroupType &reason,
          const interface::types::TimestampType &timestamp) const;

      void validateCounter(ReasonsGroupType &reason,
                           const interface::types::CounterType &counter) const;

      void validateSignatures(
          ReasonsGroupType &reason,
          const interface::types::SignatureRangeType &signatures,
          const crypto::Blob &source) const;

      void validateQueryPayloadMeta(ReasonsGroupType &reason,
                              const interface::QueryPayloadMeta &meta) const;

      void validateDescription(
          ReasonsGroupType &reason,
          const interface::types::DescriptionType &description) const;

      void validateHeight(
          ReasonsGroupType &reason,
          const interface::types::HeightType &height) const;

     private:
      const static std::string account_name_pattern_;
      const static std::string asset_name_pattern_;
      const static std::string domain_pattern_;
      const static std::string ip_v4_pattern_;
      const static std::string peer_address_pattern_;
      const static std::string account_id_pattern_;
      const static std::string asset_id_pattern_;
      const static std::string detail_key_pattern_;
      const static std::string role_id_pattern_;

      std::regex account_name_regex_;
      std::regex asset_name_regex_;
      std::regex domain_regex_;
      std::regex ip_v4_regex_;
      std::regex peer_address_regex_;
      std::regex account_id_regex_;
      std::regex asset_id_regex_;
      std::regex detail_key_regex_;
      std::regex role_id_regex_;

      // gap for future transactions
      time_t future_gap_;
      // max-delay between tx creation and validation
      static constexpr auto max_delay =
          std::chrono::hours(24) / std::chrono::milliseconds(1);
      // default value for future_gap field of FieldValidator
      static constexpr auto default_future_gap =
          std::chrono::minutes(5) / std::chrono::milliseconds(1);

      // size of key
      static const size_t public_key_size;
      static const size_t value_size;
      static const size_t description_size;
    };
  }  // namespace validation
}  // namespace shared_model

#endif  // IROHA_SHARED_MODEL_FIELD_VALIDATOR_HPP
