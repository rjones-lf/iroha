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

#ifndef IROHA_SIGNABLE_HPP
#define IROHA_SIGNABLE_HPP

#include <boost/optional.hpp>

#include "cryptography/hash_providers/sha3_256.hpp"
#include "interfaces/common_objects/signable_hash.hpp"
#include "interfaces/common_objects/signature.hpp"
#include "interfaces/common_objects/types.hpp"
#include "utils/string_builder.hpp"

namespace shared_model {

  namespace crypto {
    class Signed;
    class PublicKey;
  }  // namespace crypto

  namespace interface {

#ifdef DISABLE_BACKWARD
#define SIGNABLE(Model) Signable<Model>
#else
#define SIGNABLE(Model) Signable<Model, iroha::model::Model>
#endif

/**
 * Interface provides signatures and adds them to model object
 * @tparam Model - your new style model
 */
#ifndef DISABLE_BACKWARD
    template <typename Model,
              typename OldModel,
              typename HashProvider = shared_model::crypto::Sha3_256>
    class Signable : public Primitive<Model, OldModel> {
#else
    template <typename Model,
              typename HashProvider = shared_model::crypto::Sha3_256>
    class Signable : public ModelPrimitive<Model> {
#endif
     public:
      using HashProviderType = HashProvider;

      /**
       * @return attached signatures
       */
      virtual const SignatureSetType &signatures() const = 0;

      /**
       * Attach signature to object
       * @param signature - signature object for insertion
       * @return true, if signature was added
       */
      virtual bool addSignature(const crypto::Signed &signed_blob,
                                const crypto::PublicKey &public_key) = 0;

      /**
       * Clear object's signatures
       * @return true, if signatures were cleared
       */
      virtual bool clearSignatures() = 0;

      /**
       * @return time of creation
       */
      virtual types::TimestampType createdTime() const = 0;

      /**
       * @return object payload (everything except signatures)
       */
      virtual const types::BlobType &payload() const = 0;

      /**
       * @return blob representation of object include signatures
       */
      virtual const types::BlobType &blob() const = 0;

      /**
       * Provides comparison based on equality of objects and signatures.
       * @param rhs - another model object
       * @return true, if objects totally equal
       */
      bool operator==(const Model &rhs) const override {
        return this->hash() == rhs.hash()
            and this->signatures() == rhs.signatures()
            and this->createdTime() == rhs.createdTime();
      }

      const types::HashType &hash() const {
        if (hash_ == boost::none) {
          hash_.emplace(HashProviderType::makeHash(payload()));
        }
        return *hash_;
      }

      // ------------------------| Primitive override |-------------------------

      std::string toString() const override {
        return detail::PrettyStringBuilder()
            .init("Signable")
            .append("created_time", std::to_string(createdTime()))
            .appendAll(signatures(),
                       [](auto &signature) { return signature->toString(); })
            .finalize();
      }

     private:
      mutable boost::optional<types::HashType> hash_;
    };

  }  // namespace interface
}  // namespace shared_model

#endif  // IROHA_SIGNABLE_HPP
