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

#ifndef IROHA_SHARED_MODEL_QUERY_VALIDATOR_HPP
#define IROHA_SHARED_MODEL_QUERY_VALIDATOR_HPP

#include <boost/variant/static_visitor.hpp>

#include "interfaces/queries/query.hpp"
#include "validators/answer.hpp"

namespace shared_model {
  namespace validation {

    /**
     * Visitor used by query validator to validate each concrete query
     * @tparam FieldValidator - field validator type
     */
    template <typename FieldValidator>
    class QueryValidatorVisitor
        : public boost::static_visitor<ReasonsGroupType> {
     public:
      QueryValidatorVisitor(const FieldValidator &validator = FieldValidator())
          : validator_(validator) {}

      ReasonsGroupType operator()(
          const detail::PolymorphicWrapper<interface::GetAccount> &qry) const {
        ReasonsGroupType reason;
        reason.first = "GetAccount";

        validator_.validateAccountId(reason, qry->accountId());

        return reason;
      }

      ReasonsGroupType operator()(
          const detail::PolymorphicWrapper<interface::GetSignatories> &qry)
          const {
        ReasonsGroupType reason;
        reason.first = "GetSignatories";

        validator_.validateAccountId(reason, qry->accountId());

        return reason;
      }

      ReasonsGroupType operator()(
          const detail::PolymorphicWrapper<interface::GetAccountTransactions>
              &qry) const {
        ReasonsGroupType reason;
        reason.first = "GetAccountTransactions";

        validator_.validateAccountId(reason, qry->accountId());

        return reason;
      }

      ReasonsGroupType operator()(
          const detail::PolymorphicWrapper<
              interface::GetAccountAssetTransactions> &qry) const {
        ReasonsGroupType reason;
        reason.first = "GetAccountAssetTransactions";

        validator_.validateAccountId(reason, qry->accountId());
        validator_.validateAssetId(reason, qry->assetId());

        return reason;
      }

      ReasonsGroupType operator()(
          const detail::PolymorphicWrapper<interface::GetTransactions> &qry)
          const {
        ReasonsGroupType reason;
        reason.first = "GetTransactions";

        return reason;
      }

      ReasonsGroupType operator()(
          const detail::PolymorphicWrapper<interface::GetAccountAssets> &qry)
          const {
        ReasonsGroupType reason;
        reason.first = "GetAccountAssets";

        validator_.validateAccountId(reason, qry->accountId());
        validator_.validateAssetId(reason, qry->assetId());

        return reason;
      }

      ReasonsGroupType operator()(
          const detail::PolymorphicWrapper<interface::GetAccountDetail> &qry)
          const {
        ReasonsGroupType reason;
        reason.first = "GetAccountDetail";

        validator_.validateAccountId(reason, qry->accountId());

        return reason;
      }

      ReasonsGroupType operator()(
          const detail::PolymorphicWrapper<interface::GetRoles> &qry) const {
        ReasonsGroupType reason;
        reason.first = "GetRoles";

        return reason;
      }

      ReasonsGroupType operator()(
          const detail::PolymorphicWrapper<interface::GetRolePermissions> &qry)
          const {
        ReasonsGroupType reason;
        reason.first = "GetRolePermissions";

        validator_.validateRoleId(reason, qry->roleId());

        return reason;
      }

      ReasonsGroupType operator()(
          const detail::PolymorphicWrapper<interface::GetAssetInfo> &qry)
          const {
        ReasonsGroupType reason;
        reason.first = "GetAssetInfo";

        validator_.validateAssetId(reason, qry->assetId());

        return reason;
      }

     private:
      FieldValidator validator_;
    };

    /**
     * Class that validates query field from query
     * @tparam FieldValidator - field validator type
     * @tparam QueryFieldValidator - concrete query validator type
     */
    template <typename FieldValidator, typename QueryFieldValidator>
    class QueryValidator {
     public:
      QueryValidator(const FieldValidator &field_validator = FieldValidator(),
                     const QueryFieldValidator &query_field_validator =
                         QueryFieldValidator())
          : field_validator_(field_validator),
            query_field_validator_(query_field_validator) {}

      /**
       * Applies validation to given query
       * @param qry - query to validate
       * @return Answer containing found error if any
       */
      Answer validate(const interface::Query &qry) const {
        Answer answer;
        std::string qry_reason_name = "Query";
        ReasonsGroupType qry_reason(qry_reason_name, GroupedReasons());

        field_validator_.validateCreatorAccountId(qry_reason,
                                                  qry.creatorAccountId());
        field_validator_.validateCreatedTime(qry_reason, qry.createdTime());
        field_validator_.validateCounter(qry_reason, qry.queryCounter());

        if (not qry_reason.second.empty()) {
          answer.addReason(std::move(qry_reason));
        }

        auto reason = boost::apply_visitor(query_field_validator_, qry.get());
        if (not reason.second.empty()) {
          answer.addReason(std::move(reason));
        }

        return answer;
      }

     private:
      Answer answer_;
      FieldValidator field_validator_;
      QueryFieldValidator query_field_validator_;
    };
  }  // namespace validation
}  // namespace shared_model

#endif  // IROHA_SHARED_MODEL_QUERY_VALIDATOR_HPP
