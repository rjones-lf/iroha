/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_BUILDERS_TEST_FIXTURE_HPP
#define IROHA_BUILDERS_TEST_FIXTURE_HPP

#include <gtest/gtest.h>
#include "module/shared_model/builders/common_objects/common.hpp"

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
