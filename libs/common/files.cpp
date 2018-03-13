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

#include "common/files.hpp"

#include <boost/filesystem.hpp>

#include "logger/logger.hpp"

void iroha::remove_dir_contents(const std::string &dump_dir) {
  auto log = logger::log("common::remove_all");
  boost::system::error_code error_code;

  bool exists = boost::filesystem::exists(dump_dir, error_code);
  if (error_code != boost::system::errc::success) {
    log->error(error_code.message());
    return;
  }
  if (not exists) {
    log->error("Directory does not exist {}", dump_dir);
    return;
  }

  bool is_dir = boost::filesystem::is_directory(dump_dir, error_code);
  if (error_code != boost::system::errc::success) {
    log->error(error_code.message());
    return;
  }
  if (not is_dir) {
    log->error("{} is not a directory", dump_dir);
    return;
  }

  for (auto entry : boost::filesystem::directory_iterator(dump_dir)) {
    boost::filesystem::remove_all(entry.path(), error_code);
    if (error_code != boost::system::errc::success)
      log->error(error_code.message());
  }
}
