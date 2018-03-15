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
