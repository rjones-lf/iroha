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

#ifndef IROHA_VISITOR_HPP
#define IROHA_VISITOR_HPP

#include <boost/variant/apply_visitor.hpp>  // for boost::apply_visitor
#include <type_traits>                      // for std::decay
#include <utility>                          // for std::forward

namespace iroha {

  template <typename... Lambdas>
  struct lambda_visitor;

  template <typename Lambda, typename... Lambdas>
  struct lambda_visitor<Lambda, Lambdas...>
      : public Lambda, public lambda_visitor<Lambdas...> {
    using Lambda::operator();
    using lambda_visitor<Lambdas...>::operator();

    lambda_visitor(Lambda lambda, Lambdas... lambdas)
        : Lambda(lambda), lambda_visitor<Lambdas...>(lambdas...) {}
  };

  template <typename Lambda>
  struct lambda_visitor<Lambda> : public Lambda {
    using Lambda::operator();

    lambda_visitor(Lambda lambda) : Lambda(lambda) {}
  };

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
   * @param lambdas
   * @return visitor
   */
  template <class... Fs>
  constexpr auto make_visitor(Fs &&... fs) {
    using visitor_type = lambda_visitor<std::decay_t<Fs>...>;
    return visitor_type(std::forward<Fs>(fs)...);
  }

  /**
   * @brief Inplace visitor for boost::variant.
   * @code
   *   boost::variant<int, std::string> value = "1234";
   *   ...
   *   visit_in_place(value,
   *                  [](int v) { std::cout << "(int)" << v; },
   *                  [](std::string v) { std::cout << "(string)" << v;}
   *                  );
   * @nocode
   *
   * @param variant
   * @param lambdas
   * @param lambdas
   */
  template <typename TVariant, typename... TVisitors>
  constexpr auto visit_in_place(TVariant &&variant, TVisitors &&... visitors) {
    auto visitor = make_visitor(visitors...);
    return boost::apply_visitor(visitor, std::forward<TVariant>(variant));
  }
}  // namespace iroha

#endif  // IROHA_VISITOR_HPP
