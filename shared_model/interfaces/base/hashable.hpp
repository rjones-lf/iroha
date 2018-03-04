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

#include <boost/optional.hpp>

#include "cryptography/hash_providers/sha3_256.hpp"
#include "interfaces/base/primitive.hpp"
#include "interfaces/common_objects/types.hpp"
#include "utils/lazy_initializer.hpp"

#ifdef DISABLE_BACKWARD
#define HASHABLE_WITH_OLD(Model, OldModel) Hashable<Model>
#else
#define HASHABLE_WITH_OLD(Model, OldModel) Hashable<Model, OldModel>
#endif
#define HASHABLE(Model) HASHABLE_WITH_OLD(Model, iroha::model::Model)

namespace shared_model {
  namespace interface {
#ifdef DISABLE_BACKWARD
    template <typename ModelType,
              typename HashProvider = shared_model::crypto::Sha3_256>
    class Hashable : public ModelPrimitive<ModelType>
#else
    template <typename ModelType,
              typename OldModel,
              typename HashProvider = shared_model::crypto::Sha3_256>
    class Hashable : public Primitive<ModelType, OldModel>
#endif
    {
     public:
      using HashProviderType = HashProvider;

      /**
       * @return hash of object.
       */
      virtual const types::HashType &hash() const {
        if (hash_ == boost::none) {
          hash_.emplace(HashProvider::makeHash(blob()));
        }
        return *hash_;
      }

      /**
       * @return blob representation of object
       */
      virtual const types::BlobType &blob() const = 0;

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
      mutable boost::optional<types::HashType> hash_;
    };
  }  // namespace interface
}  // namespace shared_model
#endif  // IROHA_HASHABLE_HPP
