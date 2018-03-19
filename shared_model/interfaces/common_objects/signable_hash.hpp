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

#ifndef IROHA_SHARED_MODEL_SIGNABLE_HASH_HPP
#define IROHA_SHARED_MODEL_SIGNABLE_HASH_HPP

#include <boost/functional/hash.hpp>
#include <unordered_set>

#include "interfaces/common_objects/signature.hpp"
#include "interfaces/common_objects/types.hpp"

namespace shared_model {

  namespace interface {
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
  }  // namespace interface
}  // namespace shared_model

#endif  // IROHA_SHARED_MODEL_SIGNABLE_HASH_HPP
