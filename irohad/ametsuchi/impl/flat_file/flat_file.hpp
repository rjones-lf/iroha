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
#include <string>
#include <vector>

#include <boost/optional.hpp>
#include <nudb/nudb.hpp>

#include "logger/logger.hpp"

namespace iroha {
  namespace ametsuchi {

    /**
     * Solid storage based on raw files
     */
    class FlatFile {
     public:
      ~FlatFile() = default;

      // ----------| public API |----------

      /**
       * Type of storage key
       */
      using Identifier = uint32_t;

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
      const std::string &directory() const;

      /**
       * @return maximal not null key
       */
      Identifier last_id() const;

      bool dropAll();

      // ----------| modify operations |----------

      FlatFile(const FlatFile &rhs) = delete;

      FlatFile(FlatFile &&rhs) = delete;

      FlatFile &operator=(const FlatFile &rhs) = delete;

      FlatFile &operator=(FlatFile &&rhs) = delete;

      static constexpr size_t kFirstBlockAt{1};

     private:
      //< arbitrary number, app-specific
      static constexpr size_t appid_{0x1337u};

      //< load factor for basket
      static constexpr float load_factor_{0.5f};

      static uint32_t countBlocks(nudb::store &db, nudb::error_code &ec);

      static bool initDirectory(const std::string &path);

      std::unique_ptr<nudb::store> db_;

      /**
       * Last written key
       */
      std::atomic<Identifier> current_id_{0};

      std::string path_;

      logger::Logger log_;

      /**
       * Serializes uint32 as 4-byte big-endian array.
       * @param t a number
       * @return array of 4 bytes
       */
      static std::array<uint8_t, sizeof(uint32_t)> serialize_uint32(uint32_t t);

      FlatFile(std::unique_ptr<nudb::store> db,
               const std::string &path,
               uint32_t current);
    };
  }  // namespace ametsuchi
}  // namespace iroha
#endif  // IROHA_FLAT_FILE_HPP
