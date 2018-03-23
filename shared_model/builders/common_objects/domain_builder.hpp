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

#ifndef IROHA_DOMAIN_BUILDER_HPP
#define IROHA_DOMAIN_BUILDER_HPP

#include "builders/common_objects/common.hpp"
#include "interfaces/common_objects/domain.hpp"
#include "interfaces/common_objects/types.hpp"

namespace shared_model {
  namespace builder {
    template <typename BuilderImpl, typename Validator>
    class DomainBuilder : public CommonObjectBuilder<interface::Domain,
                                                     BuilderImpl,
                                                     Validator> {
     public:
      DomainBuilder defaultRole(
          const interface::types::RoleIdType &default_role) {
        DomainBuilder copy(*this);
        copy.builder_ = this->builder_.defaultRole(default_role);
        return copy;
      }

      DomainBuilder domainId(const interface::types::DomainIdType &domain_id) {
        DomainBuilder copy(*this);
        copy.builder_ = this->builder_.domainId(domain_id);
        return copy;
      }

     protected:
      virtual std::string builderName() const override {
        return "Domain Builder";
      }

      virtual validation::ReasonsGroupType validate(
          const interface::Domain &object) override {
        validation::ReasonsGroupType reasons;
        this->validator_.validateDomainId(reasons, object.domainId());
        this->validator_.validateRoleId(reasons, object.defaultRole());

        return reasons;
      }
    };
  }  // namespace builder
}  // namespace shared_model

#endif  // IROHA_DOMAIN_BUILDER_HPP
