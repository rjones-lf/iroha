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

#include "ametsuchi/impl/block_storage_nudb.hpp"

#include <boost/filesystem.hpp>
#include <iomanip>

namespace iroha {
  namespace ametsuchi {

    namespace fs = boost::filesystem;
    namespace sys = boost::system;
    using Identifier = BlockStorage::Identifier;

    /** implementation **/

    bool BlockStorageNuDB::init_directory(const std::string &path) {
      auto log_ = logger::log("BlockStorageNuDB::init_directory()");

      // first, check if directory exists. if not -- create.
      sys::error_code err;
      if (fs::exists(path)) {
        if (not fs::is_directory(path, err)) {
          log_->error("path {} is a file: {}", path, err.message());
          return false;
        }
      } else {
        // dir does not exist, so then create
        if (not fs::create_directory(path, err)) {
          log_->error(
              "can not create storage dir: {}\n{}", path, err.message());
          return false;
        }
      }

      return true;
    }

    boost::optional<std::unique_ptr<BlockStorageNuDB>> BlockStorageNuDB::create(
        const std::string &path) {
      auto log_ = logger::log("BlockStorageNuDB");

      if (!init_directory(path)) {
        return boost::none;
      }

      // paths to NuDB files
      fs::path dat = fs::path{path} / "iroha.dat";
      fs::path key = fs::path{path} / "iroha.key";
      fs::path log = fs::path{path} / "iroha.log";

      // try to open NuDB database
      nudb::error_code ec;
      auto db = std::make_unique<nudb::store>();
      db->open(dat.string(), key.string(), log.string(), ec);
      if (ec) {
        // remove error message
        ec.clear();

        log_->info("no database at {}, creating new", path);

        // then no database is there. create new database.
        nudb::create<nudb::xxhasher>(dat.string(),
                                     key.string(),
                                     log.string(),
                                     BlockStorageNuDB::appid_,
                                     nudb::make_salt(),
                                     sizeof(Identifier),
                                     nudb::block_size("."),
                                     BlockStorageNuDB::load_factor_,
                                     ec);
        if (ec) {
          log_->critical("can not create NuDB database: {}", ec.message());
          return boost::none;
        }

        // and open again
        db->open(dat.string(), key.string(), log.string(), ec);
        if (ec) {
          log_->critical("can not open NuDB database: {}", ec.message());
          return boost::none;
        }
      }

      log_->info("database at {} has been successfully opened", path);

      // clear error message
      ec.clear();

      // begin initialization
      auto total_blocks = BlockStorageNuDB::count_blocks(*db, ec);
      // TODO(warchant): validate blocks during initialization
      if (ec != nudb::error::key_not_found) {
        // cound_blocks searches for the last empty block sequentially, and ends
        // with error "nudb::error::key_not_found". If we did not get this type
        // of error -- something bad happened.
        log_->critical("can not read database to count blocks: {}",
                       ec.message());
        return boost::none;
      }

      return std::unique_ptr<BlockStorageNuDB>(
          new BlockStorageNuDB(std::move(db), path, total_blocks));
    }

    const std::string &BlockStorageNuDB::directory() const {
      return path_;
    }

    bool BlockStorageNuDB::add(Identifier id,
                               const std::vector<uint8_t> &blob) {
      nudb::error_code ec;
      auto key = serialize_uint32(id);
      db_->insert(key.data(), blob.data(), blob.size(), ec);
      if (ec) {
        log_->error("BlockStorage::add(): {}", ec.message());
        return false;
      }
      ++total_blocks_;
      return true;
    }

    boost::optional<std::vector<uint8_t>> BlockStorageNuDB::get(
        Identifier id) const {
      nudb::error_code ec;
      boost::optional<std::vector<uint8_t>> ret;
      auto key = serialize_uint32(id);
      db_->fetch(key.data(),
                 [&ret](const void *p, size_t size) {
                   if (size == 0) {
                     ret = boost::none;
                   } else {
                     const auto *c = static_cast<const uint8_t *>(p);
                     ret = std::vector<uint8_t>{c, c + size};
                   }
                 },
                 ec);
      if (ec) {
        log_->error("BlockStorage::get(): {}", ec.message());
        return boost::none;
      }

      return ret;
    }

    bool BlockStorageNuDB::drop_all() {
      nudb::error_code ec1, ec2, ec3;

      nudb::erase_file(db_->dat_path(), ec1);
      nudb::erase_file(db_->log_path(), ec2);
      nudb::erase_file(db_->key_path(), ec3);

      return not(ec1 or ec2 or ec3);
    }

    uint32_t BlockStorageNuDB::count_blocks(nudb::store &db,
                                            nudb::error_code &ec) {
      auto log_ = logger::log("BlockStorageNuDB::count_blocks");

      BlockStorage::Identifier current = 0;

      bool found_last = false;

      do {
        auto key = serialize_uint32(current);
        db.fetch(key.data(),
                 [&found_last, &current](const void *value, size_t size) {
                   // if we read 0 bytes, then there is no such key
                   if (size == 0u) {
                     found_last = true;
                     return;
                   }

                   // go to the next key
                   current++;
                 },
                 ec);
        if (ec == nudb::error::key_not_found) {
          return current;
        } else if (ec) {
          // some other error occurred
          log_->error("{}", ec.message());
          return 0;
        }
      } while (not found_last);

      return current;
    }

    std::array<uint8_t, sizeof(uint32_t)> BlockStorageNuDB::serialize_uint32(
        uint32_t t) {
      std::array<uint8_t, sizeof(uint32_t)> b{};

      uint8_t i = 0;
      b[i++] = (t >> 24) & 0xFF;
      b[i++] = (t >> 16) & 0xFF;
      b[i++] = (t >> 8) & 0xFF;
      b[i++] = t & 0xFF;

      return b;
    }

    size_t BlockStorageNuDB::total_keys() const {
      return total_blocks_;
    }

    /** private **/

    BlockStorageNuDB::BlockStorageNuDB(std::unique_ptr<nudb::store> db,
                                       const std::string &path,
                                       uint32_t total_blocks)
        : db_(std::move(db)),
          total_blocks_(total_blocks),
          path_(path),
          log_(logger::log("BlockStorageNuDB")) {}

  }  // namespace ametsuchi
}  // namespace iroha
