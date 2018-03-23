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

#ifndef IROHA_TRANSACTION_BUILDERS_COMMON_HPP
#define IROHA_TRANSACTION_BUILDERS_COMMON_HPP

/**
 * Prepares lambda that checks if val's type is the same with T type
 * @tparam T expected type
 * @return lambda checking val's type
 */
template <typename T>
auto verifyType() {
  return [](auto val) {
    if (std::is_same<decltype(val), T>::value) {
      SUCCEED();
    } else {
      FAIL() << "obtained: " << typeid(decltype(val)).name() << std::endl
             << "expected: " << typeid(T).name() << std::endl;
    }
  };
}

#endif //IROHA_TRANSACTION_BUILDERS_COMMON_HPP
