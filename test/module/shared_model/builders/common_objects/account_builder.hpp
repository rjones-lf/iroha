/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_ACCOUNT_BUILDER_HPP
#define IROHA_ACCOUNT_BUILDER_HPP

#include "interfaces/common_objects/account.hpp"
#include "module/shared_model/builders/common_objects/common.hpp"

// TODO: 14.02.2018 nickaleks Add check for uninitialized fields IR-972

namespace shared_model {
  namespace builder {

    /**
     * AccountBuilder is a class, used for construction of Account objects
     * @tparam BuilderImpl is a type, which defines builder for implementation
     * of shared_model. Since we return abstract classes, it is necessary for
     * them to be instantiated with some concrete implementation
     * @tparam Validator is a type, whose responsibility is
     * to perform stateless validation on model fields
     */
    template <typename BuilderImpl, typename Validator>
    class DEPRECATED AccountBuilder
        : public CommonObjectBuilder<interface::Account,
                                     BuilderImpl,
                                     Validator> {
     public:
      AccountBuilder accountId(
          const interface::types::AccountIdType &account_id) {
        AccountBuilder copy(*this);
        copy.builder_ = this->builder_.accountId(account_id);
        return copy;
      }

      AccountBuilder domainId(const interface::types::DomainIdType &domain_id) {
        AccountBuilder copy(*this);
        copy.builder_ = this->builder_.domainId(domain_id);
        return copy;
      }

      AccountBuilder quorum(const interface::types::QuorumType &quorum) {
        AccountBuilder copy(*this);
        copy.builder_ = this->builder_.quorum(quorum);
        return copy;
      }

      AccountBuilder jsonData(const interface::types::JsonType &json_data) {
        AccountBuilder copy(*this);
        copy.builder_ = this->builder_.jsonData(json_data);
        return copy;
      }

     protected:
      virtual std::string builderName() const override {
        return "Account Builder";
      }

      virtual validation::ReasonsGroupType validate(
          const interface::Account &object) override {
        validation::ReasonsGroupType reasons;
        this->validator_.validateAccountId(reasons, object.accountId());
        this->validator_.validateDomainId(reasons, object.domainId());
        this->validator_.validateQuorum(reasons, object.quorum());

        return reasons;
      }
    };
  }  // namespace builder
}  // namespace shared_model

#endif  // IROHA_ACCOUNT_BUILDER_HPP
