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

#ifndef IROHA_INPLACE_VISITOR_HPP
#define IROHA_INPLACE_VISITOR_HPP

#include <boost/hana.hpp>
#include <boost/variant.hpp>

namespace iroha {

  /**
   * Inplace visitor for boost::variant.
   * Usage:
   *
   *   boost::variant<int, std::string> value = "1234";
   *   ...
   *   visit_in_place(value,
   *                  [](int v) { std::cout << "(int)" << v; },
   *                  [](std::string v) { std::cout << "(string)" << v;}
   *                  );
   */
  template <typename TVariant, typename... TVisitors>
  inline auto visit_in_place(TVariant &&variant, TVisitors &&... visitors) {
    return boost::apply_visitor(
        boost::hana::overload(std::forward<TVisitors>(visitors)...),
        std::forward<TVariant>(variant));
  }

}  // namespace iroha

#endif  // IROHA_INPLACE_VISITOR_HPP
