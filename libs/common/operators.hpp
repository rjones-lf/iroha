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

/**
 * Bind operator. If argument has value, dereferences argument and calls
 * given function, which should return wrapped value
 * operator| is used since it has to be binary and left-associative
 *
 * nonstd::optional<int> f();
 * nonstd::optional<double> g(int);
 *
 * nonstd::optional<double> d = f()
 *    | g;
 *
 * @tparam T - monadic type
 * @tparam Transform - transform function type
 * @param t - monadic value
 * @param f - function, which takes dereferenced value, and returns
 * wrapped value
 * @return monadic value, which can be of another type
 */
template <typename T, typename Transform>
auto operator|(T t, Transform f) -> decltype(f(*t)) {
  return t ? f(*t) : t;
}
