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

#ifndef IROHA_SHARED_MODEL_PROTO_ACCOUNT_HPP
#define IROHA_SHARED_MODEL_PROTO_ACCOUNT_HPP

#include "backend/protobuf/common_objects/trivial_proto.hpp"
#include "backend/protobuf/util.hpp"
#include "interfaces/common_objects/account.hpp"
#include "responses.pb.h"
#include "utils/lazy_initializer.hpp"
#include "utils/reference_holder.hpp"

namespace shared_model {
  namespace proto {
    class Account final : public CopyableProto<interface::Account,
                                               iroha::protocol::Account,
                                               Account> {
     public:
      template <typename AccountType>
      explicit Account(AccountType &&account)
          : CopyableProto(std::forward<AccountType>(account)) {}

      Account(const Account &o) : Account(o.proto_) {}

      Account(Account &&o) noexcept : Account(std::move(o.proto_)) {}

      const interface::types::AccountIdType &accountId() const override {
        return proto_->account_id();
      }

      const interface::types::DomainIdType &domainId() const override {
        return proto_->domain_id();
      }

      interface::types::QuorumType quorum() const override {
        return proto_->quorum();
      }

      const interface::types::JsonType &jsonData() const override {
        return proto_->json_data();
      }

     private:
      template <typename T>
      using Lazy = detail::LazyInitializer<T>;
    };
  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_SHARED_MODEL_PROTO_ACCOUNT_HPP
