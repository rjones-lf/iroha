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

#ifndef IROHA_PROTO_ACCOUNT_DETAIL_RESPONSE_HPP
#define IROHA_PROTO_ACCOUNT_DETAIL_RESPONSE_HPP

#include "backend/protobuf/common_objects/account_asset.hpp"
#include "backend/protobuf/common_objects/trivial_proto.hpp"
#include "interfaces/query_responses/account_detail_response.hpp"
#include "responses.pb.h"
#include "utils/lazy_initializer.hpp"
#include "utils/reference_holder.hpp"

namespace shared_model {
  namespace proto {
    class AccountDetailResponse final
        : public CopyableProto<interface::AccountDetailResponse,
                               iroha::protocol::QueryResponse,
                               AccountDetailResponse> {
     public:
      template <typename QueryResponseType>
      explicit AccountDetailResponse(QueryResponseType &&queryResponse)
          : CopyableProto(std::forward<QueryResponseType>(queryResponse)) {}

      AccountDetailResponse(const AccountDetailResponse &o)
          : AccountDetailResponse(o.proto_) {}

      AccountDetailResponse(AccountDetailResponse &&o)
          : AccountDetailResponse(std::move(o.proto_)) {}

      const DetailType &detail() const override {
        return account_detail_response_.detail();
      }

     private:
      const iroha::protocol::AccountDetailResponse &account_detail_response_{
          proto_->account_detail_response()};
    };
  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_PROTO_ACCOUNT_DETAIL_RESPONSE_HPP
