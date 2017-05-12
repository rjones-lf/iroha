/*
Copyright Soramitsu Co., Ltd. 2016 All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <utils/expected.hpp>
#include <utils/logger.hpp>

#include "config_utils.hpp"

namespace config {

namespace detail {
std::string appendSlashIfNeeded(const std::string& str) {
  if (str.empty()) {
    return std::string("/");
  }
  if (str.back() != '/') {
    return str + "/";
  }
  return str;
}

VoidHandler ensureDirectoryExists(const std::string& path) {
  // TODO: Definitely ensure directory, not file.
  struct stat info;
  if (stat(path.c_str(), &info) != 0) {
    return makeUnexpected(exception::NotFoundPathException(path));
  }
  return {};
}
}  // namespace detail

// This method's exceptions don't be caughted. (fatal error)
std::string get_iroha_home() {
  const auto iroha_home_ptr = getenv("IROHA_HOME");
  if (iroha_home_ptr == nullptr) {
    throw exception::config::UndefinedIrohaHomeException();
  }

  const auto iroha_home_with_slash =
      detail::appendSlashIfNeeded(std::string(iroha_home_ptr));

  const auto res = detail::ensureDirectoryExists(iroha_home_with_slash);
  if (!res) {
    logger::error("config") << "Invalid $IROHA_HOME";
    std::rethrow_exception(res.excptr());
  }

  return iroha_home_with_slash;
}
}  // namespace config
