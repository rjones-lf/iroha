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

#ifndef IROHA_SHARED_MODEL_SIGNATURE_HPP
#define IROHA_SHARED_MODEL_SIGNATURE_HPP

#include "cryptography/blob.hpp"
#include "cryptography/public_key.hpp"
#include "cryptography/signed.hpp"
#include "interfaces/base/model_primitive.hpp"
#include "utils/string_builder.hpp"

namespace shared_model {
  namespace interface {

    /**
     * Class represents signature of high-level domain objects.
     */
    class Signature : public ModelPrimitive<Signature> {
     public:
      /**
       * Type of public key
       */
      using PublicKeyType = crypto::PublicKey;

      /**
       * @return public key of signatory
       */
      virtual const PublicKeyType &publicKey() const = 0;

      /**
       * Type of signed data
       */
      using SignedType = crypto::Signed;

      /**
       * @return signed data
       */
      virtual const SignedType &signedData() const = 0;

      bool operator==(const Signature &rhs) const override {
        return publicKey() == rhs.publicKey();
      }

      std::string toString() const override {
        return detail::PrettyStringBuilder()
            .init("Signature")
            .append("publicKey", publicKey().hex())
            .append("signedData", signedData().hex())
            .finalize();
      }
    };
  }  // namespace interface
}  // namespace shared_model
#endif  // IROHA_SHARED_MODEL_SIGNATURE_HPP
