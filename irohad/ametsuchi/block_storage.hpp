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

#ifndef IROHA_BLOCK_STORE_HPP
#define IROHA_BLOCK_STORE_HPP

#include <boost/optional.hpp>
#include <vector>
#include <string>

namespace iroha {
  namespace ametsuchi {

    class BlockStorage {
     public:
      /**
       * Type of storage key
       */
      using Identifier = uint32_t;

      /**
       * Add entity with binary data
       * @param id - reference key
       * @param blob - data associated with key
       */
      virtual bool add(Identifier id, const std::vector<uint8_t> &blob) = 0;

      /**
       * Returns directory, where database is stored.
       */
      virtual const std::string& directory() const = 0;

      /**
       * Get data associated with
       * @param id - reference key
       * @return - blob, if exists
       */
      virtual boost::optional<std::vector<uint8_t>> get(Identifier id) const = 0;

      /**
       * @return number of identifiers in block storage.
       */
      virtual size_t total_keys() const = 0;

      /**
       * @brief Clear database.
       * @return true, if success, false otherwise.
       */
      virtual bool drop_all() = 0;
    };
  }  // namespace ametsuchi
}  // namespace iroha

#endif  // IROHA_BLOCK_STORE_HPP
