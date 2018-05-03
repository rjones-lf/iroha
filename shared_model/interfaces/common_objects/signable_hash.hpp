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

#include <unordered_set>

#include "interfaces/common_objects/signature.hpp"
#include "interfaces/common_objects/types.hpp"

namespace shared_model {

  namespace interface {
    /**
     * Property class for SignatureSetType that contains hashing and comparison
     * operations.
     */
    class SignatureSetTypeOps {
     public:
      /**
       * @param sig is item to find hash from
       * @return calculated hash of public key
       */
      size_t operator()(const types::SignatureType &sig) const {
        return std::hash<std::string>{}(sig->publicKey().hex());
      }

      /**
       * Function for set elements uniqueness by public key
       * @param lhs
       * @param rhs
       * @return true, if public keys are the same
       */
      bool operator()(const types::SignatureType &lhs,
                      const types::SignatureType &rhs) const {
        return lhs->publicKey() == rhs->publicKey();
      }
    };
  }  // namespace interface
}  // namespace shared_model

#endif  // IROHA_SHARED_MODEL_SIGNABLE_HASH_HPP
