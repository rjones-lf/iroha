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

#ifndef IROHA_SHARED_MODEL_PROTO_SIGNATORIES_RESPONSE_HPP
#define IROHA_SHARED_MODEL_PROTO_SIGNATORIES_RESPONSE_HPP

#include "interfaces/query_responses/signatories_response.hpp"

#include "backend/protobuf/common_objects/trivial_proto.hpp"
#include "cryptography/public_key.hpp"
#include "qry_responses.pb.h"
#include "utils/lazy_initializer.hpp"

namespace shared_model {
  namespace proto {
    class SignatoriesResponse final
        : public CopyableProto<interface::SignatoriesResponse,
                               iroha::protocol::QueryResponse,
                               SignatoriesResponse> {
     public:
      template <typename QueryResponseType>
      explicit SignatoriesResponse(QueryResponseType &&queryResponse);

      SignatoriesResponse(const SignatoriesResponse &o);

      SignatoriesResponse(SignatoriesResponse &&o);

      const interface::types::PublicKeyCollectionType &keys() const override;

     private:
      template <typename T>
      using Lazy = detail::LazyInitializer<T>;

      const iroha::protocol::SignatoriesResponse &signatoriesResponse_;

      const Lazy<interface::types::PublicKeyCollectionType> keys_;
    };
  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_SHARED_MODEL_PROTO_SIGNATORIES_RESPONSE_HPP
