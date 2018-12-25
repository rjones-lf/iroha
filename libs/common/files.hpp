/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_FILES_HPP
#define IROHA_FILES_HPP

#include <string>

/**
 * This source file contains common methods related to files
 */
namespace iroha {

  /**
   * Remove all files and directories inside a folder.
   * Keeps the target folder.
   * @param dump_dir - target folder
   */
  void remove_dir_contents(const std::string &dump_dir);
}  // namespace iroha
#endif  // IROHA_FILES_HPP
