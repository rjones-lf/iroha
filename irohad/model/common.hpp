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

#ifndef IROHA_COMMON_HPP
#define IROHA_COMMON_HPP

#include <memory>
#include <boost/optional.hpp>

namespace iroha {
  namespace model {
    // Optional over shared pointer
    template <typename T>
    using optional_ptr = boost::optional<std::shared_ptr<T>>;

    template <typename T, typename... Args>
    optional_ptr<T> make_optional_ptr(Args &&... args) {
      return std::make_shared<T>(std::forward<Args>(args)...);
    }
  }  // namespace model
}  // namespace iroha

#endif  // IROHA_COMMON_HPP
