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

#ifndef IROHA_FLAT_FILE_HPP
#define IROHA_FLAT_FILE_HPP

#include <atomic>
#include <memory>
#include <boost/optional.hpp>
#include <string>
#include <vector>

#include "logger/logger.hpp"

namespace iroha {
  namespace ametsuchi {

    /**
     * Solid storage based on raw files
     */
    class FlatFile {
      /**
       * Private tag used to construct unique and shared pointers
       * without new operator
       */
      struct private_tag {};

     public:
      // ----------| public API |----------

      /**
       * Type of storage key
       */
      using Identifier = uint32_t;

      static const uint32_t DIGIT_CAPACITY = 16;

      /**
       * Convert id to a string representation. The string representation is
       * always DIGIT_CAPACITY-character width regardless of the value of `id`.
       * If the length of the string representation of `id` is less than
       * DIGIT_CAPACITY, then the returned value is filled with leading zeros.
       *
       * For example, if str_rep(`id`) is "123", then the returned value is
       * "0000000000000123".
       *
       * @param id - for conversion
       * @return string repr of identifier
       */
      static std::string id_to_name(Identifier id);

      /**
       * Create storage in paths
       * @param path - target path for creating
       * @return created storage
       */
      static boost::optional<std::unique_ptr<FlatFile>> create(
          const std::string &path);

      /**
       * Add entity with binary data
       * @param id - reference key
       * @param blob - data associated with key
       */
      bool add(Identifier id, const std::vector<uint8_t> &blob);

      /**
       * Get data associated with
       * @param id - reference key
       * @return - blob, if exists
       */
      boost::optional<std::vector<uint8_t>> get(Identifier id) const;

      /**
       * @return folder of storage
       */
      std::string directory() const;

      /**
       * @return maximal not null key
       */
      Identifier last_id() const;

      /**
       * Checking consistency of storage for provided folder
       * If some block in the middle is missing all blocks following it are
       * deleted
       * @param dump_dir - folder of storage
       * @return - last available identifier
       */
      static boost::optional<Identifier> check_consistency(
          const std::string &dump_dir);

      void dropAll();

      // ----------| modify operations |----------

      FlatFile(const FlatFile &rhs) = delete;

      FlatFile(FlatFile &&rhs) = delete;

      FlatFile &operator=(const FlatFile &rhs) = delete;

      FlatFile &operator=(FlatFile &&rhs) = delete;

      // ----------| private API |----------

      /**
       * Create storage in path with respect to last key
       * @param last_id - maximal key written in storage
       * @param path - folder of storage
       */
      FlatFile(Identifier last_id,
               const std::string &path,
               FlatFile::private_tag);

     private:
      // ----------| private fields |----------

      /**
       * Last written key
       */
      std::atomic<Identifier> current_id_;

      /**
       * Folder of storage
       */
      const std::string dump_dir_;

      logger::Logger log_;

     public:
      ~FlatFile() = default;
    };
  }  // namespace ametsuchi
}  // namespace iroha
#endif  // IROHA_FLAT_FILE_HPP
