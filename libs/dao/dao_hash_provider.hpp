/*
Copyright Soramitsu Co., Ltd. 2016 All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#ifndef IROHA_DAO_HASH_PROVIDER_HPP
#define IROHA_DAO_HASH_PROVIDER_HPP

#include <common.hpp>
#include <crypto/common.hpp>
#include "dao.hpp"

namespace iroha {
  namespace dao {

    /**
     * Hash provider is an abstract factory for computing hashes on DAO objects.
     * @tparam T - length of hash
     */
    template <int T>
    class HashProvider {
     public:
      /**
       * Abstract method for computing hash on DAO: Proposal
       * @param proposal - source object for computing hash
       */
      virtual iroha::hash_t<T> get_hash(const Proposal &proposal) = 0;

      /**
       * Abstract method for computing hash on DAO: Block
       * @param block - source object for computing hash
       */
      virtual iroha::hash_t<T> get_hash(const Block &block) = 0;

      /**
       * Abstract method for computing hash  on DAO: Transaction
       * @param tx - source object for computing hash
       */
      virtual iroha::hash_t<T> get_hash(const Transaction &tx) = 0;
    };
  }
}
#endif  // IROHA_DAO_HASH_PROVIDER_HPP
