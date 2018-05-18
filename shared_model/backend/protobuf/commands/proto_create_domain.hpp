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

#include "backend/protobuf/common_objects/trivial_proto.hpp"
#include "commands.pb.h"
#include "interfaces/commands/create_domain.hpp"

namespace shared_model {
  namespace proto {

    class CreateDomain final : public CopyableProto<interface::CreateDomain,
                                                    iroha::protocol::Command,
                                                    CreateDomain> {
     public:
      template <typename CommandType>
      explicit CreateDomain(CommandType &&command);

      CreateDomain(const CreateDomain &o);

      CreateDomain(CreateDomain &&o) noexcept;

      const interface::types::DomainIdType &domainId() const override;

      const interface::types::RoleIdType &userDefaultRole() const override;

     private:
      const iroha::protocol::CreateDomain &create_domain_;
    };

  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_PROTO_CREATE_DOMAIN_HPP
