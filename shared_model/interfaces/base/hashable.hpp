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

#ifndef IROHA_HASHABLE_HPP
#define IROHA_HASHABLE_HPP

#include "cryptography/hash.hpp"
#include "cryptography/hash_providers/sha3_256.hpp"
#include "interfaces/base/primitive.hpp"
#include "utils/lazy_initializer.hpp"

namespace shared_model {
  namespace interface {
    template <typename ModelType,
              typename OldModel,
              typename HashProvider = shared_model::crypto::Sha3_256>
    class Hashable : public Primitive<ModelType, OldModel> {
     public:
      /// Type of hash
      using HashType = crypto::Hash;

      using BlobType = crypto::Blob;

      using HashProviderType = HashProvider;

      /**
       * @return hash of object.
       */
      virtual const HashType &hash() const { return *hash_; }

      /**
       * @return blob representation of object
       */
      virtual const BlobType &blob() const = 0;

      /**
       * Overriding operator== with equality hash semantics:
       * equality of hashes <=> equality of objects.
       * @param rhs - another model object
       * @return true, if hashes are equal, false otherwise
       */
      bool operator==(const ModelType &rhs) const override {
        return this->hash() == rhs.hash();
      }

     protected:
      detail::LazyInitializer<HashType> hash_ = detail::makeLazyInitializer(
          [this] { return HashProvider::makeHash(blob()); });
    };
  }  // namespace interface
}  // namespace shared_model
#endif  // IROHA_HASHABLE_HPP
