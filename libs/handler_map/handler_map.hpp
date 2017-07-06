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

#ifndef IROHA_HANDLER_MAP_HPP
#define IROHA_HANDLER_MAP_HPP

#include <functional>
#include <nonstd/optional.hpp>
#include <typeindex>
#include <unordered_map>

template <typename T, typename R>
class HandlerMap {
 public:
  template <typename U>
  HandlerMap &insert(std::function<R(const U &)> function);
  nonstd::optional<std::function<R(const T &)>> find(const T &command);

 private:
  std::unordered_map<std::type_index, std::function<R(const T &)>> handlers_;
};

template <typename T, typename R>
template <typename U>
HandlerMap<T, R> &HandlerMap<T, R>::insert(
    std::function<R(const U &)> function) {
  handlers_.insert(std::make_pair(
      std::type_index(typeid(U)), [function](const T &command) -> R {
        const U &command_ = static_cast<const U &>(command);
        return function(command_);
      }));
  return *this;
}

template <typename T, typename R>
nonstd::optional<std::function<R(const T &)>> HandlerMap<T, R>::find(
    const T &command) {
  auto it = handlers_.find(std::type_index(typeid(command)));
  return it != handlers_.end() ? nonstd::make_optional(it->second)
                               : nonstd::nullopt;
}

#endif  // IROHA_HANDLER_MAP_HPP
