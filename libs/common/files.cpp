/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "common/files.hpp"

#include <ciso646>

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
