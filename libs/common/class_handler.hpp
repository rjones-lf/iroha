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

#ifndef IROHA_CLASS_HANDLER_HPP
#define IROHA_CLASS_HANDLER_HPP

#include <typeindex>
#include <unordered_set>

/**
 * Class provides handling of classes
 * for working with them as with collection
 */
class ClassHandler {
 public:
  /**
   * Register type for further working
   * @param index - type identifier
   * @return number of registered types
   */
  size_t register_type(const std::type_index &index) {
    set.insert(index);
    return set.size();
  }

  /**
   * Provide all types registered in object
   */
  std::unordered_set<std::type_index> types() {
    return set;
  }

 private:
  std::unordered_set<std::type_index> set;
};
#endif  // IROHA_CLASS_HANDLER_HPP
