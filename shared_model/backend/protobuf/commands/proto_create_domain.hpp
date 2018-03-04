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

#ifndef IROHA_PROTO_CREATE_DOMAIN_HPP
#define IROHA_PROTO_CREATE_DOMAIN_HPP

#include "interfaces/commands/create_domain.hpp"

namespace shared_model {
  namespace proto {

    class CreateDomain final : public CopyableProto<interface::CreateDomain,
                                                    iroha::protocol::Command,
                                                    CreateDomain> {
     public:
      template <typename CommandType>
      explicit CreateDomain(CommandType &&command)
          : CopyableProto(std::forward<CommandType>(command)) {}

      CreateDomain(const CreateDomain &o) : CreateDomain(o.proto_) {}

      CreateDomain(CreateDomain &&o) noexcept
          : CreateDomain(std::move(o.proto_)) {}

      const interface::types::DomainIdType &domainId() const override {
        return create_domain_.domain_id();
      }

      const interface::types::RoleIdType &userDefaultRole() const override {
        return create_domain_.default_role();
      }

     private:
      const iroha::protocol::CreateDomain &create_domain_{
          proto_->create_domain()};
    };

  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_PROTO_CREATE_DOMAIN_HPP
