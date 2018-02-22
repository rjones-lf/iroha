/**
 * Copyright Soramitsu Co., Ltd. 2018 All Rights Reserved.
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

#ifndef IROHA_WRAPPER_HPP
#define IROHA_WRAPPER_HPP

#include "utils/polymorphic_wrapper.hpp"

namespace iroha {
  /// Project-wide container for polymorphic types
  template <typename T>
  using Wrapper = shared_model::detail::PolymorphicWrapper<T>;

  /**
   * Creates a generic wrapped type with custom-type ctor if specified
   *
   * @tparam Y is returning wrapped type
   * @tparam T is ctor that should be called on Wrapper creation
   * @tparam Args are arguments to the ctor
   * @return created wrapped instance
   *
   * note: ctor without args should be called with explicit Y AND T
   */
  template <typename Y, typename T = Y, typename... Args>
  auto makeWrapper(Args &&... args) {
    return static_cast<shared_model::detail::PolymorphicWrapper<Y>>(
        shared_model::detail::makePolymorphic<T, Args...>(
            std::forward<Args>(args)...));
  }
}  // namespace iroha

#endif  // IROHA_WRAPPER_HPP
