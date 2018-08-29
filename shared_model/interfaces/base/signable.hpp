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

#include <boost/functional/hash.hpp>
#include <boost/optional.hpp>
#include <unordered_set>

#include "cryptography/default_hash_provider.hpp"
#include "interfaces/base/model_primitive.hpp"
#include "interfaces/common_objects/signature.hpp"
#include "interfaces/common_objects/types.hpp"
#include "utils/string_builder.hpp"

namespace shared_model {

  namespace crypto {
    class Signed;
    class PublicKey;
  }  // namespace crypto

  namespace interface {

    /**
     * Interface provides signatures and adds them to model object
     * @tparam Model - your model
     */
    template <typename Model,
              typename HashProvider = shared_model::crypto::Sha3_256>
    class Signable : public ModelPrimitive<Model> {
     public:
      /**
       * @return attached signatures
       */
      virtual types::SignatureRangeType signatures() const = 0;

      /**
       * Attach signature to object
       * @param signature - signature object for insertion
       * @return true, if signature was added
       */
      virtual bool addSignature(const crypto::Signed &signed_blob,
                                const crypto::PublicKey &public_key) = 0;

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
        return equalsByValue(rhs)
            // is_permutation consumes ~O(N^2)
            and std::is_permutation(signatures().begin(),
                                    signatures().end(),
                                    rhs.signatures().begin());
      }

      /**
       * Provides comaprison based on equality objects only
       * @param rhs - another model object
       * @return true, if hashes of objects are equal
       */
      bool equalsByValue(const Model &rhs) const {
        return this->hash() == rhs.hash();
      }

      const types::HashType &hash() const {
        if (hash_ == boost::none) {
          hash_.emplace(HashProvider::makeHash(payload()));
        }
        return *hash_;
      }

      // ------------------------| Primitive override |-------------------------

      std::string toString() const override {
        return detail::PrettyStringBuilder()
            .init("Signable")
            .append("created_time", std::to_string(createdTime()))
            .appendAll(signatures(),
                       [](auto &signature) { return signature.toString(); })
            .finalize();
      }

     protected:
      /**
       * Type of set of signatures
       *
       * Note: we can't use const SignatureType due to unordered_set
       * limitations: it requires to have write access for elements for some
       * internal operations.
       */

     protected:
      class SignatureSetTypeOps {
       public:
        /**
         * @param sig is item to find hash from
         * @return calculated hash of public key
         */
        template <typename T>
        size_t operator()(const T &sig) const {
          return std::hash<std::string>{}(sig.publicKey().hex());
        }

        /**
         * Function for set elements uniqueness by public key
         * @param lhs
         * @param rhs
         * @return true, if public keys are the same
         */
        template <typename T>
        bool operator()(const T &lhs, const T &rhs) const {
          return lhs.publicKey() == rhs.publicKey();
        }
      };

      template <typename T>
      using SignatureSetType =
          std::unordered_set<T, SignatureSetTypeOps, SignatureSetTypeOps>;

     private:
      mutable boost::optional<types::HashType> hash_;
    };

  }  // namespace interface
}  // namespace shared_model

#endif  // IROHA_SIGNABLE_HPP
