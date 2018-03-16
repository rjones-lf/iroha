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
#include <boost/range/adaptor/indexed.hpp>
#include <boost/range/algorithm/find_if.hpp>
#include <iomanip>
#include <iostream>
#include <sstream>
#include "common/files.hpp"

using namespace iroha::ametsuchi;
using Identifier = FlatFile::Identifier;

// ----------| public API |----------

std::string FlatFile::id_to_name(Identifier id) {
  std::ostringstream os;
  os << std::setw(FlatFile::DIGIT_CAPACITY) << std::setfill('0') << id;
  return os.str();
}

boost::optional<std::unique_ptr<FlatFile>> FlatFile::create(
    const std::string &path) {
  auto log_ = logger::log("FlatFile::create()");

  boost::system::error_code err;
  if (not boost::filesystem::is_directory(path, err)
      and not boost::filesystem::create_directory(path, err)) {
    log_->error("Cannot create storage dir: {}\n{}", path, err.message());
    return boost::none;
  }

  auto res = FlatFile::check_consistency(path);
  return std::make_unique<FlatFile>(*res, path, private_tag{});
}

bool FlatFile::add(Identifier id, const std::vector<uint8_t> &block) {
  // TODO(x3medima17): Change bool to generic Result return type

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

boost::optional<std::vector<uint8_t>> FlatFile::get(Identifier id) const {
  const auto filename =
      boost::filesystem::path{dump_dir_} / FlatFile::id_to_name(id);
  if (not boost::filesystem::exists(filename)) {
    log_->info("get({}) file not found", id);
    return boost::none;
  }
  const auto fileSize = boost::filesystem::file_size(filename);
  std::vector<uint8_t> buf;
  buf.resize(fileSize);
  boost::filesystem::ifstream file(filename, std::ifstream::binary);
  if (not file.is_open()) {
    log_->info("get({}) problem with opening file", id);
    return boost::none;
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
  iroha::remove_dir_contents(dump_dir_);
  auto res = FlatFile::check_consistency(dump_dir_);
  current_id_.store(*res);
}

// ----------| private API |----------

FlatFile::FlatFile(Identifier current_id,
                   const std::string &path,
                   FlatFile::private_tag)
    : dump_dir_(path) {
  log_ = logger::log("FlatFile");
  current_id_.store(current_id);
}

boost::optional<Identifier> FlatFile::check_consistency(
    const std::string &dump_dir) {
  auto log = logger::log("FLAT_FILE");

  if (dump_dir.empty()) {
    log->error("check_consistency({}), not directory", dump_dir);
    return boost::none;
  }

  auto const files = [&dump_dir] {
    std::vector<boost::filesystem::path> ps;
    std::copy(boost::filesystem::directory_iterator{dump_dir},
              boost::filesystem::directory_iterator{},
              std::back_inserter(ps));
    std::sort(ps.begin(), ps.end(), std::less<boost::filesystem::path>());
    return ps;
  }();

  auto const missing = boost::range::find_if(
      files | boost::adaptors::indexed(1), [](const auto &it) {
        return FlatFile::id_to_name(it.index()) != it.value().filename();
      });

  std::for_each(
      missing.get(), files.cend(), [](const boost::filesystem::path &p) {
        boost::filesystem::remove(p);
      });

  return missing.get() - files.cbegin();
}
