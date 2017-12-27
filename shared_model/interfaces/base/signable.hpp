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
#include <unordered_set>
#include "interfaces/base/hashable.hpp"
#include "interfaces/common_objects/signature.hpp"
#include "interfaces/common_objects/types.hpp"
#include "utils/polymorphic_wrapper.hpp"
#include "utils/string_builder.hpp"

namespace shared_model {
  namespace interface {

    /**
     * Interface provides signatures and adding them to model object
     * @tparam Model - your new style model
     * Architecture note: we inherit Signable from Hashable with following
     * assumption - all Signable objects are signed by hash value.
     */
    template <typename Model, typename OldModel>
    class Signable : public Hashable<Model, OldModel> {
     public:
      /**
       * Hash class for SigWrapper type. It's required since std::unordered_set
       * uses hash inside and it should be declared explicitly for user-defined
       * types.
       */
      class SignableHash {
       public:
        /**
         * Operator which actually calculates hash. Uses boost::hash_combine to
         * calculate hash from several fields.
         * @param sig - item to find hash from
         * @return calculated hash
         */
        size_t operator()(const types::SignatureType &sig) const {
          std::size_t seed = 0;
          boost::hash_combine(seed, sig->publicKey().blob());
          boost::hash_combine(seed, sig->signedData().blob());
          return seed;
        }
      };

      /**
       * Type of set of signatures
       *
       * Note: we can't use const SignatureType due to unordered_set
       * limitations: it requires to have write access for elements for some
       * internal operations.
       */
      using SignatureSetType =
          std::unordered_set<types::SignatureType, SignableHash>;

      /**
       * @return attached signatures
       */
      virtual const SignatureSetType &signatures() const = 0;

      /**
       * Attach signature to object
       * @param signature - signature object for insertion
       * @return true, if signature was added
       */
      virtual bool addSignature(const types::SignatureType &signature) = 0;

      /**
       * @return time of creation
       */
      virtual types::TimestampType createdTime() const = 0;

      /**
       * @return object payload (everything except signatures)
       */
      virtual const typename Hashable<Model, OldModel>::BlobType &payload()
          const = 0;

      /**
       * Provides comparison based on equality of objects and signatures.
       * @param rhs - another model object
       * @return true, if objects totally equal
       */
      virtual bool equals(const Model &rhs) const {
        return *this == rhs and this->signatures() == rhs.signatures()
            and this->createdTime() == rhs.createdTime();
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
    };

  }  // namespace interface
}  // namespace shared_model

#endif  // IROHA_SIGNABLE_HPP
