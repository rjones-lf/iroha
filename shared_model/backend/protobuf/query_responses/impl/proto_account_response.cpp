/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "backend/protobuf/query_responses/proto_account_response.hpp"

namespace shared_model {
  namespace proto {

    template <typename QueryResponseType>
    AccountResponse::AccountResponse(QueryResponseType &&queryResponse)
        : CopyableProto(std::forward<QueryResponseType>(queryResponse)),
          accountResponse_{proto_->account_response()},
          accountRoles_{[this] {
            return boost::accumulate(
                accountResponse_.account_roles(),
                AccountRolesIdType{},
                [](auto &&roles, const auto &role) {
                  roles.push_back(interface::types::RoleIdType(role));
                  return std::move(roles);
                });
          }},
          account_{[this] { return Account(accountResponse_.account()); }} {}

    template AccountResponse::AccountResponse(AccountResponse::TransportType &);
    template AccountResponse::AccountResponse(
        const AccountResponse::TransportType &);
    template AccountResponse::AccountResponse(
        AccountResponse::TransportType &&);

    AccountResponse::AccountResponse(const AccountResponse &o)
        : AccountResponse(o.proto_) {}

    AccountResponse::AccountResponse(AccountResponse &&o)
        : AccountResponse(std::move(o.proto_)) {}

    const interface::Account &AccountResponse::account() const {
      return *account_;
    }

    const AccountResponse::AccountRolesIdType &AccountResponse::roles() const {
      return *accountRoles_;
    }

  }  // namespace proto
}  // namespace shared_model
