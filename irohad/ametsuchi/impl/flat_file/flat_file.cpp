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

#include "ametsuchi/impl/flat_file/flat_file.hpp"

#include <boost/filesystem.hpp>

namespace iroha {
  namespace ametsuchi {

    namespace fs = boost::filesystem;
    namespace sys = boost::system;
    using Identifier = FlatFile::Identifier;

    /** implementation **/

    bool FlatFile::initDirectory(const std::string &path) {
      auto log_ = logger::log("init_directory()");

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
          log_->error("can't create storage dir: {}\n{}", path, err.message());
          return false;
        }
      }

      return true;
    }

    boost::optional<std::unique_ptr<FlatFile>> FlatFile::create(
        const std::string &path) {
      auto log_ = logger::log("FlatFile");

      if (not FlatFile::initDirectory(path)) {
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
                                     FlatFile::appid_,
                                     nudb::make_salt(),
                                     sizeof(Identifier),
                                     nudb::block_size("."),
                                     FlatFile::load_factor_,
                                     ec);
        if (ec) {
          log_->critical("can't create NuDB database: {}", ec.message());
          return boost::none;
        }

        // and open again
        db->open(dat.string(), key.string(), log.string(), ec);
        if (ec) {
          log_->critical("can't open NuDB database: {}", ec.message());
          return boost::none;
        }
      }

      log_->info("database at {} has been successfully opened", path);

      // clear error message
      ec.clear();

      // begin initialization
      auto total_blocks = FlatFile::countBlocks(*db, ec);
      // TODO(warchant): IR-1017 validate blocks during initialization
      if (ec != nudb::error::key_not_found) {
        // cound_blocks searches for the last empty block sequentially, and ends
        // with error "nudb::error::key_not_found". If we did not get this type
        // of error -- something bad happened.
        log_->critical("can not read database to count blocks: {}",
                       ec.message());
        return boost::none;
      }

      auto last_id = total_blocks == 0u ? 0u : total_blocks - 1;

      return std::unique_ptr<FlatFile>(
          new FlatFile(std::move(db), path, last_id));
    }

    const std::string &FlatFile::directory() const {
      return path_;
    }

    bool FlatFile::add(Identifier id, const std::vector<uint8_t> &blob) {
      nudb::error_code ec;
      auto key = serialize_uint32(id);
      db_->insert(key.data(), blob.data(), blob.size(), ec);
      if (ec) {
        log_->error("add(): {}", ec.message());
        return false;
      }
      current_id_.store(id);
      return true;
    }

    boost::optional<std::vector<uint8_t>> FlatFile::get(Identifier id) const {
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
        log_->error("get(): {}", ec.message());
        return boost::none;
      }

      return ret;
    }

    bool FlatFile::dropAll() {
      int ret = 0;

      auto try_erase = [this](const std::string &path) {
        nudb::error_code ec;
        nudb::erase_file(path, ec);
        if (ec) {
          log_->error("can't erase file {}, reason: {}", path, ec.message());
          return 1;
        }
        return 0;
      };

      ret |= try_erase(db_->key_path());
      ret |= try_erase(db_->log_path());
      ret |= try_erase(db_->dat_path());

      current_id_.store(0);

      return ret == 0;
    }

    uint32_t FlatFile::countBlocks(nudb::store &db, nudb::error_code &ec) {
      auto log_ = logger::log("FlatFile::count_blocks");

      FlatFile::Identifier current = kFirstBlockAt;

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

    std::array<uint8_t, sizeof(uint32_t)> FlatFile::serialize_uint32(
        uint32_t t) {
      std::array<uint8_t, sizeof(uint32_t)> b{};

      uint8_t i = 0;
      b[i++] = (t >> 24) & 0xFF;
      b[i++] = (t >> 16) & 0xFF;
      b[i++] = (t >> 8) & 0xFF;
      b[i++] = t & 0xFF;

      return b;
    }

    Identifier FlatFile::last_id() const {
      return current_id_;
    }

    /** private **/

    FlatFile::FlatFile(std::unique_ptr<nudb::store> db,
                       const std::string &path,
                       uint32_t current)
        : db_(std::move(db)),
          current_id_(current),
          path_(path),
          log_(logger::log("FlatFile")) {}

  }  // namespace ametsuchi
}  // namespace iroha
