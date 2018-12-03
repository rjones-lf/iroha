/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_SHARED_MODEL_PROTO_ACCOUNT_RESPONSE_HPP
#define IROHA_SHARED_MODEL_PROTO_ACCOUNT_RESPONSE_HPP

#include <boost/range/numeric.hpp>

#include "backend/protobuf/common_objects/account.hpp"
#include "backend/protobuf/common_objects/trivial_proto.hpp"
#include "interfaces/query_responses/account_response.hpp"
#include "qry_responses.pb.h"
#include "utils/lazy_initializer.hpp"

namespace shared_model {
  namespace proto {
    class AccountResponse final
        : public CopyableProto<interface::AccountResponse,
                               iroha::protocol::QueryResponse,
                               AccountResponse> {
     public:
      template <typename QueryResponseType>
      explicit AccountResponse(QueryResponseType &&queryResponse);

      AccountResponse(const AccountResponse &o);

      AccountResponse(AccountResponse &&o);

      const interface::Account &account() const override;

      const AccountRolesIdType &roles() const override;

     private:
      template <typename T>
      using Lazy = detail::LazyInitializer<T>;

      const iroha::protocol::AccountResponse &accountResponse_;

      const Lazy<AccountRolesIdType> accountRoles_;

      const Lazy<shared_model::proto::Account> account_;
    };
  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_SHARED_MODEL_PROTO_ACCOUNT_RESPONSE_HPP
