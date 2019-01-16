/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_SIGNATURE_BUILDER_HPP
#define IROHA_SIGNATURE_BUILDER_HPP

#include "interfaces/common_objects/signature.hpp"
#include "interfaces/common_objects/types.hpp"
#include "module/shared_model/builders/common_objects/common.hpp"

// TODO: 14.02.2018 nickaleks Add check for uninitialized fields IR-972

namespace shared_model {
  namespace builder {

    /**
     * SignatureBuilder is a class, used for construction of Signature objects
     * @tparam BuilderImpl is a type, which defines builder for implementation
     * of shared_model. Since we return abstract classes, it is necessary for
     * them to be instantiated with some concrete implementation
     * @tparam Validator is a type, whose responsibility is
     * to perform stateless validation on model fields
     */
    template <typename BuilderImpl, typename Validator>
    class DEPRECATED SignatureBuilder
        : public CommonObjectBuilder<interface::Signature,
                                     BuilderImpl,
                                     Validator> {
     public:
      SignatureBuilder publicKey(
          const shared_model::interface::types::PubkeyType &key) {
        SignatureBuilder copy(*this);
        copy.builder_ = this->builder_.publicKey(key);
        return copy;
      }

      SignatureBuilder signedData(
          const interface::Signature::SignedType &signed_data) {
        SignatureBuilder copy(*this);
        copy.builder_ = this->builder_.signedData(signed_data);
        return copy;
      }

     protected:
      virtual std::string builderName() const override {
        return "Signature Builder";
      }

      virtual validation::ReasonsGroupType validate(
          const interface::Signature &object) override {
        validation::ReasonsGroupType reasons;
        this->validator_.validatePubkey(reasons, object.publicKey());

        return reasons;
      }
    };
  }  // namespace builder
}  // namespace shared_model

#endif  // IROHA_SIGNATURE_BUILDER_HPP
