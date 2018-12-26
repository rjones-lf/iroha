/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_FLAT_FILE_HPP
#define IROHA_FLAT_FILE_HPP

#include "ametsuchi/key_value_storage.hpp"

#include <atomic>
#include <memory>

#include "logger/logger.hpp"

namespace iroha {
  namespace ametsuchi {

    /**
     * Solid storage based on raw files
     */
    class FlatFile : public KeyValueStorage {
      /**
       * Private tag used to construct unique and shared pointers
       * without new operator
       */
      struct private_tag {};

     public:
      // ----------| public API |----------

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

      bool add(Identifier id, const Bytes &blob) override;

      boost::optional<Bytes> get(Identifier id) const override;

      std::string directory() const override;

      Identifier last_id() const override;

      /**
       * Checking consistency of storage for provided folder
       * If some block in the middle is missing all blocks following it are
       * deleted
       * @param dump_dir - folder of storage
       * @return - last available identifier
       */
      static boost::optional<Identifier> check_consistency(
          const std::string &dump_dir);

      void dropAll() override;

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
       * @param log to print progress
       */
      FlatFile(Identifier last_id,
               const std::string &path,
               FlatFile::private_tag,
               logger::Logger log = logger::log("FlatFile"));

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
