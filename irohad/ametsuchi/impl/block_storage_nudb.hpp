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

#ifndef IROHA_BLOCK_STORAGE_NUDB_HPP
#define IROHA_BLOCK_STORAGE_NUDB_HPP

#include "ametsuchi/block_storage.hpp"

#include <array>
#include <nudb/nudb.hpp>
#include "logger/logger.hpp"

namespace iroha {
  namespace ametsuchi {

    class BlockStorageNuDB : public BlockStorage {
     public:
      /**
       * Creates instance of BlockStorageNuDB in `path`
       * @param path path to the directory, where block store is created
       * @return boost::none in case of error, uptr to BlockStorageNuDB
       * otherwise
       */
      static boost::optional<std::unique_ptr<BlockStorageNuDB>> create(
          const std::string &path);

      /**
       * Store blob by id (value by key).
       * @param id - key
       * @param blob - value
       * @return true if success, false otherwise.
       */
      bool add(BlockStorage::Identifier id,
               const std::vector<uint8_t> &blob) override;

      /**
       *
       */

      /**
       * Initialize database directory given path.
       * @param path path in filesystem to the directory.
       * @return false if path is a file or no directory can be created, true
       * otherwise.
       */
      static bool init_directory(const std::string &path);

      /**
       * Fetch value by key.
       * @param id - key
       * @return blob if success (value), boost::none otherwise
       */
      boost::optional<std::vector<uint8_t>> get(
          BlockStorage::Identifier id) const override;

      /**
       * Returns total number of keys starting from 0.
       *
       * @example in case of 5 keys, then latest id is 4.
       */
      size_t total_keys() const override;

      /**
       * Removes database.
       * @return true if success, false otherwise.
       */
      bool drop_all() override;

      /**
       * Returns path to the storage directory.
       */
      const std::string &directory() const override;

      //< arbitrary number, app-specific
      static constexpr size_t appid_{0x1337u};

      //< load factor for basket
      static constexpr float load_factor_{0.5f};

     private:
      /**
       * Some sort of initialization: it counts number of existent keys in
       * database starting with 0.
       * @param db initialized database
       * @param ec internal error code
       * @return number of non-empty keys.
       */
      static uint32_t count_blocks(nudb::store &db, nudb::error_code &ec);

      /**
       * Serializes uint32 as 4-byte big-endian array.
       * @param t a number
       * @return array of 4 bytes
       */
      static std::array<uint8_t, sizeof(uint32_t)> serialize_uint32(uint32_t t);

      BlockStorageNuDB(std::unique_ptr<nudb::store> db,
                       const std::string &path,
                       uint32_t total_blocks);

      std::unique_ptr<nudb::store> db_;

      //< total number of blocks in database
      uint32_t total_blocks_;
      std::string path_;
      logger::Logger log_;
    };

  }  // namespace ametsuchi
}  // namespace iroha

#endif  // IROHA_BLOCK_STORAGE_NUDB_HPP
