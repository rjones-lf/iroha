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
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/range/adaptor/indexed.hpp>
#include <boost/range/algorithm/find_if.hpp>
#include <iomanip>
#include <sstream>
#include "common/files.hpp"

using namespace iroha::ametsuchi;

namespace {
  const uint32_t DIGIT_CAPACITY = 16;

  /**
   * Convert id to a string representation. The string representation is always
   * DIGIT_CAPACITY-character width regardless of the value of `id`.
   * If the length of the string representation of `id` is less than
   * DIGIT_CAPACITY, then the returned value is filled with leading zeros.
   *
   * For example, if str_rep(`id`) is "123", then the returned value is
   * "0000000000000123".
   *
   * @param id - for conversion
   * @return string repr of identifier
   */
  std::string id_to_name(Identifier id) {
    std::ostringstream os;
    os << std::setw(DIGIT_CAPACITY) << std::setfill('0') << id;
    return os.str();
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

    auto const missing = boost::range::find_if(
        files | boost::adaptors::indexed(1), [](const auto &it) {
          return id_to_name(it.index()) != it.value().filename();
        });

    std::for_each(
        missing.get(), files.cend(), [](const boost::filesystem::path &p) {
          boost::filesystem::remove(p);
        });

    return missing.get() - files.cbegin();
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
  return std::make_unique<FlatFile>(*res, path, private_tag{});
}

bool FlatFile::add(Identifier id, const std::vector<uint8_t> &block) {
  if (id != current_id_ + 1) {
    log_->warn("Cannot append non-consecutive block");
    return false;
  }

  auto next_id = id;
  const auto file_name = boost::filesystem::path{dump_dir_} / id_to_name(id);

  // Write block to binary file
  if (boost::filesystem::exists(file_name)) {
    // File already exist
    log_->warn("insertion for {} failed, because file already exists", id);
    return false;
  }
  // New file will be created
  boost::filesystem::ofstream file(file_name.native(), std::ofstream::binary);
  if (not file.is_open()) {
    log_->warn("Cannot open file by index {} for writing", id);
    return false;
  }

  auto val_size =
      sizeof(std::remove_reference<decltype(block)>::type::value_type);

  file.write(reinterpret_cast<const char *>(block.data()),
             block.size() * val_size);

  // Update internals, release lock
  current_id_ = next_id;
  return true;
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

FlatFile::FlatFile(Identifier current_id, const std::string &path, FlatFile::private_tag)
    : dump_dir_(path) {
  log_ = logger::log("FlatFile");
  current_id_.store(current_id);
}
