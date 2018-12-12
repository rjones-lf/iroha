/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "bindings/model_query_builder.hpp"

#include <gtest/gtest.h>

/**
 * @given Model transaction builder
 * @when build() is called on builder without set fields
 * @then Exception is thrown
 */
TEST(ModelQueryBuilderTest, EmptyBuilder) {
  auto query = shared_model::bindings::ModelQueryBuilder();

  EXPECT_ANY_THROW(query.build());
}
