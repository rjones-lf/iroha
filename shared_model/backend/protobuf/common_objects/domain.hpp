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

#ifndef IROHA_SHARED_MODEL_PROTO_DOMAIN_HPP
#define IROHA_SHARED_MODEL_PROTO_DOMAIN_HPP

#include "backend/protobuf/common_objects/trivial_proto.hpp"
#include "interfaces/common_objects/domain.hpp"
#include "responses.pb.h"
#include "utils/lazy_initializer.hpp"
#include "utils/reference_holder.hpp"

namespace shared_model {
  namespace proto {
    class Domain final : public CopyableProto<interface::Domain,
                                              iroha::protocol::Domain,
                                              Domain> {
     public:
      template <typename DomainType>
      explicit Domain(DomainType &&domain)
          : CopyableProto(std::forward<DomainType>(domain)) {}

      Domain(const Domain &o) : Domain(o.proto_) {}

      Domain(Domain &&o) noexcept : Domain(std::move(o.proto_)) {}

      const interface::types::DomainIdType &domainId() const override {
        return proto_->domain_id();
      }

      const interface::types::RoleIdType &defaultRole() const override {
        return proto_->default_role();
      }
    };
  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_SHARED_MODEL_PROTO_DOMAIN_HPP
