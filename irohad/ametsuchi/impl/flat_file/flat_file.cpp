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
#include <boost/filesystem.hpp>
#include <fstream>
#include "common/files.hpp"

using namespace iroha::ametsuchi;

const uint32_t DIGIT_CAPACITY = 16;

// TODO 19/08/17 Muratov rework separator with platform independent approach
// IR-495 #goodfirstissue
const std::string SEPARATOR = "/";

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

  using namespace boost::filesystem;

  auto const files = [&dump_dir] {
    std::vector<path> ps;
    std::copy(directory_iterator{dump_dir},
              directory_iterator{},
              std::back_inserter(ps));
    std::sort(ps.begin(), ps.end(),[](const path& lhs, const path& rhs){
        return lhs.compare(rhs) < 0;
      });
    return ps;
  }();

  auto const missing =
      std::find_if(files.cbegin(), files.cend(), [id = 0](const path &p) mutable {
        ++id;
        return id_to_name(id) != p.filename();
      });
  std::for_each(missing, files.cend(), [](const path &p) {
    remove(p);
  });
  return missing - files.cbegin();
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

  // TODO 19/08/17 Muratov change creating folder with system independent
  // approach IR-496 #goodfirstissue
  if (mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1) {
    if (errno != EEXIST) {
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
  auto file_name = dump_dir_ + SEPARATOR + id_to_name(id);

  // Write block to binary file
  if (boost::filesystem::exists(file_name)) {
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
  if (not boost::filesystem::exists(filename)) {
    log_->info("get({}) file not found", id);
    return nonstd::nullopt;
  }
  auto fileSize = file_size(filename);
  std::vector<uint8_t> buf;
  buf.resize(fileSize);
  std::ifstream file(filename, std::ifstream::binary);
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
