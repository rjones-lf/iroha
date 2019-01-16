/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_DOMAIN_BUILDER_HPP
#define IROHA_DOMAIN_BUILDER_HPP

#include "interfaces/common_objects/domain.hpp"
#include "interfaces/common_objects/types.hpp"
#include "module/shared_model/builders/common_objects/common.hpp"

namespace shared_model {
  namespace builder {
    template <typename BuilderImpl, typename Validator>
    class DEPRECATED DomainBuilder
        : public CommonObjectBuilder<interface::Domain,
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
