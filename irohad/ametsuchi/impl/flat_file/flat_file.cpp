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

#include "ametsuchi/impl/flat_file/flat_file.hpp"
#include <array>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include "common/files.hpp"

using namespace iroha::ametsuchi;

namespace {
  const uint32_t DIGIT_CAPACITY = 16;

  /**
   * Convert id to string repr
   * @param id - for conversion
   * @return string repr of identifier
   */
  std::string id_to_name(Identifier id) {
    std::array<char, DIGIT_CAPACITY + 1> buf;
    std::snprintf(buf.begin(), buf.size(), "%016u", id);
    // drop the trailing null character written by snprintf.
    return {buf.begin(), buf.end() - 1};
  }

  /**
   * Checking consistency of storage for provided folder
   * If some block in the middle is missing all blocks following it are deleted
   * @param dump_dir - folder of storage
   * @return - last available identifier
   */
  nonstd::optional<Identifier> check_consistency(const std::string &dump_dir) {
    auto log = logger::log("FLAT_FILE");

    if (dump_dir.empty()) {
      log->error("check_consistency({}), not directory", dump_dir);
      return nonstd::nullopt;
    }

    auto const files = [&dump_dir] {
      std::vector<boost::filesystem::path> ps;
      std::copy(boost::filesystem::directory_iterator{dump_dir},
                boost::filesystem::directory_iterator{},
                std::back_inserter(ps));
      std::sort(ps.begin(),
                ps.end(),
                [](const boost::filesystem::path &lhs,
                   const boost::filesystem::path &rhs) {
                  return lhs.compare(rhs) < 0;
                });
      return ps;
    }();

    auto const missing =
        std::find_if(files.cbegin(),
                     files.cend(),
                     [id = 0](const boost::filesystem::path &p) mutable {
                       ++id;
                       return id_to_name(id) != p.filename();
                     });
    std::for_each(missing, files.cend(), [](const boost::filesystem::path &p) {
      boost::filesystem::remove(p);
    });
    return missing - files.cbegin();
  }
}  // namespace

// ----------| public API |----------

std::unique_ptr<FlatFile> FlatFile::create(const std::string &path) {
  auto log_ = logger::log("FlatFile::create()");

  if (boost::filesystem::create_directory(path)) {
    if (!boost::filesystem::is_directory(path)) {
      log_->error("Cannot create storage dir: {}", path);
    }
  }
  auto res = check_consistency(path);
  if (not res) {
    log_->error("Checking consistency for {} - failed", path);
    return nullptr;
  }
  return std::unique_ptr<FlatFile>(new FlatFile(*res, path));
}

void FlatFile::add(Identifier id, const std::vector<uint8_t> &block) {
  if (id != current_id_ + 1) {
    log_->warn("Cannot append non-consecutive block");
    return;
  }

  auto next_id = id;
  const auto file_name = boost::filesystem::path{dump_dir_} / id_to_name(id);

  // Write block to binary file
  if (boost::filesystem::exists(file_name)) {
    // File already exist
    log_->warn("insertion for {} failed, because file already exists", id);
    return;
  }
  // New file will be created
  boost::filesystem::ofstream file(file_name.native(), std::ofstream::binary);
  if (not file.is_open()) {
    log_->warn("Cannot open file by index {} for writing", id);
    return;
  }

  auto val_size =
      sizeof(std::remove_reference<decltype(block)>::type::value_type);

  file.write(reinterpret_cast<const char *>(block.data()),
             block.size() * val_size);

  // Update internals, release lock
  current_id_ = next_id;
}

nonstd::optional<std::vector<uint8_t>> FlatFile::get(Identifier id) const {
  const auto filename = boost::filesystem::path{dump_dir_} / id_to_name(id);
  if (not boost::filesystem::exists(filename)) {
    log_->info("get({}) file not found", id);
    return nonstd::nullopt;
  }
  const auto fileSize = boost::filesystem::file_size(filename);
  std::vector<uint8_t> buf;
  buf.resize(fileSize);
  boost::filesystem::ifstream file(filename, std::ifstream::binary);
  if (not file.is_open()) {
    log_->info("get({}) problem with opening file", id);
    return nonstd::nullopt;
  }
  file.read(reinterpret_cast<char *>(buf.data()), fileSize);
  return buf;
}

std::string FlatFile::directory() const {
  return dump_dir_;
}

Identifier FlatFile::last_id() const {
  return current_id_.load();
}

void FlatFile::dropAll() {
  remove_all(dump_dir_);
  auto res = check_consistency(dump_dir_);
  current_id_.store(*res);
}

// ----------| private API |----------

FlatFile::FlatFile(Identifier current_id, const std::string &path)
    : dump_dir_(path) {
  log_ = logger::log("FlatFile");
  current_id_.store(current_id);
}
