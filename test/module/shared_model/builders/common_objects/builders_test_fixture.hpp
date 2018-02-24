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

#ifndef IROHA_BUILDERS_TEST_FIXTURE_HPP
#define IROHA_BUILDERS_TEST_FIXTURE_HPP

#include <gtest/gtest.h>
#include "builders/common_objects/common.hpp"

/**
 * Perform testFunc on two objects of type std::shared_ptr<T> which are taken
 * from Result. If at least one of the results contains error then test fails
 * with error message
 */
template <typename T, typename TestFunc>
void testResultObjects(shared_model::builder::BuilderResult<T> &a,
                       shared_model::builder::BuilderResult<T> &b,
                       TestFunc t) {
  auto result = a | [&](auto object1) {
    return b.match(
        [&](typename shared_model::builder::BuilderResult<T>::ValueType &v) {
          t(object1, v.value);
          return shared_model::builder::BuilderResult<T>(v);
        },
        [](typename shared_model::builder::BuilderResult<T>::ErrorType &e) {
          return shared_model::builder::BuilderResult<T>(e);
        });
  };
  result.match(
      [&](const typename shared_model::builder::BuilderResult<T>::ValueType
              &v) {},
      [](const typename shared_model::builder::BuilderResult<T>::ErrorType &e) {
        FAIL() << *e.error;
      });
}

#endif  // IROHA_BUILDERS_TEST_FIXTURE_HPP
