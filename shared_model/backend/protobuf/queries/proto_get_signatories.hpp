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

#ifndef IROHA_PROTO_GET_SIGNATORIES_H
#define IROHA_PROTO_GET_SIGNATORIES_H

#include "interfaces/queries/get_signatories.hpp"

#include "queries.pb.h"
#include "utils/lazy_initializer.hpp"
#include "utils/reference_holder.hpp"

namespace shared_model {
  namespace proto {
    class GetSignatories final : public CopyableProto<interface::GetSignatories,
                                                      iroha::protocol::Query,
                                                      GetSignatories> {
     public:
      template <typename QueryType>
      explicit GetSignatories(QueryType &&query)
          : CopyableProto(std::forward<QueryType>(query)),
            account_signatories_(detail::makeReferenceGenerator(
                &proto_->payload(),
                &iroha::protocol::Query::Payload::get_account_signatories)) {}

      GetSignatories(const GetSignatories &o) : GetSignatories(o.proto_) {}

      GetSignatories(GetSignatories &&o) noexcept
          : GetSignatories(std::move(o.proto_)) {}

      const interface::types::AccountIdType &accountId() const override {
        return account_signatories_->account_id();
      }

     private:
      // ------------------------------| fields |-------------------------------

      template <typename T>
      using Lazy = detail::LazyInitializer<T>;

      const Lazy<const iroha::protocol::GetSignatories &> account_signatories_;
    };

  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_PROTO_GET_SIGNATORIES_H
