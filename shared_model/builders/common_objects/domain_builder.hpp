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
    class DomainBuilder {
     public:
      BuilderResult<shared_model::interface::Domain> build() {
        auto domain = builder_.build();
        shared_model::validation::ReasonsGroupType reasons(
            "Domain Builder", shared_model::validation::GroupedReasons());
        shared_model::validation::Answer answer;
        validator_.validateRoleId(reasons, domain.defaultRole());
        validator_.validateDomainId(reasons, domain.domainId());

        if (!reasons.second.empty()) {
          answer.addReason(std::move(reasons));
          return iroha::expected::makeError(
              std::make_shared<std::string>(answer.reason()));
        }
        std::shared_ptr<shared_model::interface::Domain> domain_ptr(domain.copy());
        return iroha::expected::makeValue(domain_ptr);
      }

      DomainBuilder &defaultRole(
          const interface::types::RoleIdType &default_role) {
        builder_ = builder_.defaultRole(default_role);
        return *this;
      }

      DomainBuilder &domainId(const interface::types::DomainIdType &domain_id) {
        builder_ = builder_.domainId(domain_id);
        return *this;
      }

     private:
      Validator validator_;
      BuilderImpl builder_;
    };
  }  // namespace builder
}  // namespace shared_model

#endif  // IROHA_DOMAIN_BUILDER_HPP
