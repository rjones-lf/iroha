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

#pragma once

namespace detail {

  template <typename... Lambdas>
  struct lambda_visitor;

  template <typename Lambda1, typename... Lambdas>
  struct lambda_visitor<Lambda1, Lambdas...>
      : public Lambda1, public lambda_visitor<Lambdas...> {
    using Lambda1::operator();
    using lambda_visitor<Lambdas...>::operator();

    lambda_visitor(Lambda1 l1, Lambdas... lambdas)
        : Lambda1(l1), lambda_visitor<Lambdas...>(lambdas...) {}
  };

  template <typename Lambda1>
  struct lambda_visitor<Lambda1> : public Lambda1 {
    using Lambda1::operator();

    lambda_visitor(Lambda1 l1) : Lambda1(l1) {}
  };
}

/**
 * @brief Convenient in-place compile-time visitor creation, from a set of
 * lambdas
 *
 * @code
 * make_visitor([](int a){ return 1; },
 *              [](std::string b) { return 2; });
 * @nocode
 *
 * is essentially the same as
 *
 * @code
 * struct visitor : public boost::static_visitor<int> {
 *   int operator()(int a) { return 1; }
 *   int operator()(std::string b) { return 2; }
 * }
 * @nocode
 *
 * @tparam Fs
 * @param fs
 * @return
 */
template <class... Fs>
constexpr auto make_visitor(Fs &&... fs) {
  using visitor_type = detail::lambda_visitor<std::decay_t<Fs>...>;
  return visitor_type(std::forward<Fs>(fs)...);
}
