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

#ifndef IROHA_SHARED_MODEL_PROTO_ACCOUNT_RESPONSE_HPP
#define IROHA_SHARED_MODEL_PROTO_ACCOUNT_RESPONSE_HPP

#include <boost/range/numeric.hpp>

#include "backend/protobuf/common_objects/account.hpp"
#include "backend/protobuf/common_objects/trivial_proto.hpp"
#include "interfaces/query_responses/account_response.hpp"
#include "responses.pb.h"
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
