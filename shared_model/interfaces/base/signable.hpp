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

#include "interfaces/base/hashable.hpp"
#include "interfaces/common_objects/signable_hash.hpp"
#include "interfaces/common_objects/signature.hpp"
#include "interfaces/common_objects/types.hpp"
#include "utils/polymorphic_wrapper.hpp"
#include "utils/string_builder.hpp"

namespace shared_model {
  namespace interface {

#ifdef DISABLE_BACKWARD
#define SIGNABLE(Model) Signable<Model>
#else
#define SIGNABLE(Model) Signable<Model, iroha::model::Model>
#endif

/**
 * Interface provides signatures and adds them to model object
 * @tparam Model - your new style model
 * Architecture note: we inherit Signable from Hashable with following
 * assumption - all Signable objects are signed by hash value.
 */

#ifndef DISABLE_BACKWARD
    template <typename Model, typename OldModel>
    class Signable : public Hashable<Model, OldModel> {
#else
    template <typename Model>
    class Signable : public Hashable<Model> {
#endif
     public:
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
       * Provides comparison based on equality of objects and signatures.
       * @param rhs - another model object
       * @return true, if objects totally equal
       */
      virtual bool equals(const Model &rhs) const {
        return *this == rhs and this->signatures() == rhs.signatures()
            and this->createdTime() == rhs.createdTime();
      }

#ifndef DISABLE_BACKWARD
      const typename types::HashType &hash() const override {
        if (Hashable<Model, OldModel>::hash_ == boost::none) {
          Hashable<Model, OldModel>::hash_.emplace(
              Hashable<Model, OldModel>::HashProviderType::makeHash(payload()));
        }
        return *Hashable<Model, OldModel>::hash_;
      }
#else
      const typename types::HashType &hash() const override {
        if (Hashable<Model>::hash_ == boost::none) {
          Hashable<Model>::hash_.emplace(
              Hashable<Model>::HashProviderType::makeHash(payload()));
        }
        return *Hashable<Model>::hash_;
      }
#endif

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
