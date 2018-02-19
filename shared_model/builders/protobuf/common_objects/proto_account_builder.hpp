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

#ifndef IROHA_PROTO_ACCOUNT_BUILDER_HPP
#define IROHA_PROTO_ACCOUNT_BUILDER_HPP

#include "backend/protobuf/common_objects/account.hpp"
#include "responses.pb.h"
#include "utils/polymorphic_wrapper.hpp"

namespace shared_model {
  namespace proto {

    /**
     * AccountBuilder is used to construct Account proto objects with initialized
     * protobuf implementation
     */
    class AccountBuilder {
     public:
      shared_model::proto::Account build() {
        return shared_model::proto::Account(std::move(account_));
      }

      AccountBuilder &accountId(const interface::types::AccountIdType &account_id) {
        account_.set_account_id(account_id);
        return *this;
      }

      AccountBuilder &domainId(const interface::types::DomainIdType &domain_id) {
        account_.set_domain_id(domain_id);
        return *this;
      }

      AccountBuilder &quorum(const interface::types::QuorumType &quorum) {
        account_.set_quorum(quorum);
        return *this;
      }

      AccountBuilder &jsonData(const interface::types::JsonType &json_data) {
        account_.set_json_data(json_data);
        return *this;
      }

     private:
      iroha::protocol::Account account_;
    };
  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_PROTO_ACCOUNT_BUILDER_HPP
