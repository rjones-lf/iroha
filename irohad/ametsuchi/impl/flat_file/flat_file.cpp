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
#include <dirent.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fstream>
#include <iostream>

using namespace iroha::ametsuchi;

const uint32_t DIGIT_CAPACITY = 16;

// todo rework separator with platform independent approach
const std::string SEPARATOR = "/";

// TODO(@warchant): move these functions to libs/util/filesystem

/**
 * Convert id to string repr
 * @param id - for conversion
 * @return string repr of identifier
 */
std::string id_to_name(Identifier id) {
  std::string new_id(DIGIT_CAPACITY, '\0');
  std::sprintf(&new_id[0], "%016u", id);
  return new_id;
}

/**
 * Convert string to identifier
 * @param name - string for conversion
 * @return numeric identifier
 */
Identifier name_to_id(const std::string &name) {
  std::string::size_type sz;
  return static_cast<Identifier>(std::stoul(name, &sz));
}

/**
 * Check if file exists
 * @param name - full path to file
 * @return true, if exists
 */
bool file_exist(const std::string &name) {
  struct stat buffer {};
  return (stat(name.c_str(), &buffer) == 0);
}

/**
 * Remove file from folder
 * @param dump_dir - target dir
 * @param id - identifier of file
 */
void remove(const std::string &dump_dir, std::string filename) {
  auto f_name = dump_dir + SEPARATOR + filename;

  if (std::remove(f_name.c_str()) != 0) {
    logger::log("FLAT_FILE")
        ->error("remove({}, {}): error on deleting file", dump_dir, filename);
  }
}

/**
 * Checking consistency of storage for provided folder
 * @param dump_dir - folder of storage
 * @return - last available identifier
 */
nonstd::optional<Identifier> check_consistency(const std::string &dump_dir) {
  auto log = logger::log("FLAT_FILE");

  Identifier tmp_id = 0u;
  if (dump_dir.empty()) {
    log->error("check_consistency({}), not directory", dump_dir);
    return nonstd::nullopt;
  }

  // TODO(@warchant): refactor with boost::filesystem
  // Directory iterator:
  struct dirent **namelist;
  auto status = scandir(dump_dir.c_str(), &namelist, nullptr, alphasort);
  if (status < 0) {
    log->error("check_consistency({}), scandir error: {}", dump_dir, status);
    return nonstd::nullopt;
  }
  if (status < 3) {
    log->info("check_consistency({}), directory is empty", dump_dir);
    return 0;
  }

  auto n = static_cast<uint32_t>(status);
  tmp_id++;
  for (auto i = 2u; i < n; ++i) {
    if (id_to_name(tmp_id) != namelist[i]->d_name) {
      for (auto j = i; j < n; ++j) {
        remove(dump_dir, namelist[j]->d_name);
      }
      break;
    }
    tmp_id = name_to_id(namelist[i]->d_name);
  }

  for (auto j = 0u; j < n; ++j) {
    free(namelist[j]);
  }
  free(namelist);

  return tmp_id;
}

/**
 * Compute size of file in bytes
 * @param filename - file for processing
 * @return number of bytes contains in file
 */
long file_size(const std::string &filename) {
  struct stat stat_buf {};
  int rc = stat(filename.c_str(), &stat_buf);
  return rc == 0 ? stat_buf.st_size : 0u;
}

// ----------| public API |----------

std::unique_ptr<FlatFile> FlatFile::create(const std::string &path) {
  auto log_ = logger::log("FlatFile::create()");

  // TODO (@warchant): refactor with boost::filesystem
  if (mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1) {
    if (errno != EEXIST) {
      log_->error("Cannot create storage dir: {}, reason: {}", path,
                  strerror(errno));
    }
  }

  auto res = check_consistency(path);
  if (!res) {
    log_->error("Checking consistency for {} - failed", path);
    return nullptr;
  }

  return std::unique_ptr<FlatFile>(new FlatFile(*res, path));
}

void FlatFile::add(Identifier id, const std::vector<uint8_t> &block) {
  auto next_id = id;
  auto file_name = dump_dir_ + SEPARATOR + id_to_name(id);

  // Write block to binary file
  if (file_exist(file_name)) {
    // File already exist
    log_->warn("insertion for {} failed, because file already exists", id);
    return;
  }
  // New file will be created
  std::ofstream file(file_name, std::ofstream::binary);
  if (not file.is_open()) {
    log_->warn("Cannot open file by index {} for writing", id);
  }

  auto val_size =
      sizeof(std::remove_reference<decltype(block)>::type::value_type);

  file.write(reinterpret_cast<const char *>(block.data()),
             block.size() * val_size);

  // Update internals, release lock
  current_id_ = next_id;
}

nonstd::optional<std::vector<uint8_t>> FlatFile::get(Identifier id) const {
  std::string filename = dump_dir_ + SEPARATOR + id_to_name(id);
  if (not file_exist(filename)) {
    log_->info("get({}) file not found", id);
    return nonstd::nullopt;
  }

  auto fileSize = file_size(filename);
  std::vector<uint8_t> buf(fileSize);

  std::ifstream file(filename, std::ifstream::binary);
  if (not file.is_open()) {
    log_->info("get({}) problem with opening file", id);
    return nonstd::nullopt;
  }

  file.read(reinterpret_cast<char *>(buf.data()), fileSize);
  return buf;
}

std::string FlatFile::directory() const { return dump_dir_; }

Identifier FlatFile::last_id() const { return current_id_.load(); }

// ----------| private API |----------

FlatFile::FlatFile(Identifier current_id, const std::string &path)
    : dump_dir_(path), log_(logger::log("FlatFile")) {
  current_id_.store(current_id);
}
