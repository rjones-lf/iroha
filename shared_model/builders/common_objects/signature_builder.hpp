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

#ifndef IROHA_SIGNATURE_BUILDER_HPP
#define IROHA_SIGNATURE_BUILDER_HPP

#include "builders/common_objects/common.hpp"
#include "interfaces/common_objects/signature.hpp"
#include "interfaces/common_objects/types.hpp"

namespace shared_model {
  namespace builder {
    template <typename BuilderImpl, typename Validator>
    class SignatureBuilder {
     public:
      BuilderResult<shared_model::interface::Signature> build() {
        auto signature = builder_.build();
        shared_model::validation::ReasonsGroupType reasons(
            "Signature Builder", shared_model::validation::GroupedReasons());
        shared_model::validation::Answer answer;
        validator_.validatePubkey(reasons, signature.publicKey());

        if (!reasons.second.empty()) {
          answer.addReason(std::move(reasons));
          return iroha::expected::makeError(
              std::make_shared<std::string>(answer.reason()));
        }
        std::shared_ptr<shared_model::interface::Signature> signature_ptr(signature.copy());
        return iroha::expected::makeValue(
            shared_model::detail::PolymorphicWrapper<
                shared_model::interface::Signature>(signature_ptr));
      }

      SignatureBuilder &publicKey(const shared_model::interface::types::PubkeyType &key) {
        builder_ = builder_.publicKey(key);
        return *this;
      }

      SignatureBuilder &signedData(
          const interface::Signature::SignedType &signed_data) {
        builder_ = builder_.signedData(signed_data);
        return *this;
      }

     private:
      Validator validator_;
      BuilderImpl builder_;
    };
  }  // namespace builder
}  // namespace shared_model

#endif  // IROHA_SIGNATURE_BUILDER_HPP
