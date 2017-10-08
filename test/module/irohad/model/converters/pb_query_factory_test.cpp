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

#include "model/converters/pb_query_factory.hpp"
#include <gtest/gtest.h>
#include "crypto/hash.hpp"
#include "model/generators/query_generator.hpp"

#include "model/queries/get_asset_info.hpp"
#include "model/queries/get_roles.hpp"

using namespace iroha::model::converters;
using namespace iroha::model::generators;
using namespace iroha::model;

void runQueryTest(std::shared_ptr<Query> query) {
  PbQueryFactory queryFactory;
  auto pb_query = queryFactory.serialize(query);
  ASSERT_TRUE(pb_query.has_value());
  auto res_query = queryFactory.deserialize(pb_query.value());
  ASSERT_TRUE(res_query.has_value());
  // TODO: overload operator == for queries and replace with it
  ASSERT_EQ(iroha::hash(*res_query.value()), iroha::hash(*query));
}

TEST(PbQueryFactoryTest, SerializeGetAccount) {
  PbQueryFactory queryFactory;
  QueryGenerator queryGenerator;
  auto query = queryGenerator.generateGetAccount(0, "123", 0, "test");
  auto pb_query = queryFactory.serialize(query);
  ASSERT_TRUE(pb_query.has_value());
  auto res_query = queryFactory.deserialize(pb_query.value());
  ASSERT_TRUE(res_query.has_value());
  // TODO: overload operator == for queries and replace with it
  ASSERT_EQ(iroha::hash(*res_query.value()), iroha::hash(*query));
}

TEST(PbQueryFactoryTest, SerializeGetAccountAssets) {
  PbQueryFactory queryFactory;
  QueryGenerator queryGenerator;
  auto query =
      queryGenerator.generateGetAccountAssets(0, "123", 0, "test", "coin");
  auto pb_query = queryFactory.serialize(query);
  ASSERT_TRUE(pb_query.has_value());
  auto res_query = queryFactory.deserialize(pb_query.value());
  ASSERT_TRUE(res_query.has_value());
  // TODO: overload operator == for queries and replace with it
  ASSERT_EQ(iroha::hash(*res_query.value()), iroha::hash(*query));
}

TEST(PbQueryFactoryTest, SerializeGetAccountTransactions) {
  PbQueryFactory queryFactory;
  QueryGenerator queryGenerator;
  auto query =
      queryGenerator.generateGetAccountTransactions(0, "123", 0, "test");
  auto pb_query = queryFactory.serialize(query);
  ASSERT_TRUE(pb_query.has_value());
  auto res_query = queryFactory.deserialize(pb_query.value());
  ASSERT_TRUE(res_query.has_value());
  // TODO: overload operator == for queries and replace with it
  ASSERT_EQ(iroha::hash(*res_query.value()), iroha::hash(*query));
}

TEST(PbQueryFactoryTest, SerializeGetAccountTransactionsWithPager) {
  PbQueryFactory queryFactory;
  QueryGenerator queryGenerator;
  auto query = queryGenerator.generateGetAccountTransactionsWithPager(
      0, "123", 0, "test", Pager{iroha::hash256_t{}, 1});
  auto pb_query = queryFactory.serialize(query);
  ASSERT_TRUE(pb_query.has_value());
  auto res_query = queryFactory.deserialize(pb_query.value());
  ASSERT_TRUE(res_query.has_value());
  // TODO: overload operator == for queries and replace with it
  ASSERT_EQ(iroha::hash(*res_query.value()), iroha::hash(*query));
}

TEST(PbQueryFactoryTest, SerializeGetAccountAssetsTransactionsWithPager) {
  PbQueryFactory queryFactory;
  QueryGenerator queryGenerator;
  auto query = queryGenerator.generateGetAccountAssetsTransactionsWithPager(
      0, "123", 0, "test", {"a", "b"}, Pager{iroha::hash256_t{}, 1});
  auto pb_query = queryFactory.serialize(query);
  ASSERT_TRUE(pb_query.has_value());
  auto res_query = queryFactory.deserialize(pb_query.value());
  ASSERT_TRUE(res_query.has_value());
  // TODO: overload operator == for queries and replace with it
  ASSERT_EQ(iroha::hash(*res_query.value()), iroha::hash(*query));
}

TEST(PbQueryFactoryTest, SerializeGetSignatories) {
  PbQueryFactory queryFactory;
  QueryGenerator queryGenerator;
  auto query = queryGenerator.generateGetSignatories(0, "123", 0, "test");
  auto pb_query = queryFactory.serialize(query);
  ASSERT_TRUE(pb_query.has_value());
  auto res_query = queryFactory.deserialize(pb_query.value());
  ASSERT_TRUE(res_query.has_value());
  // TODO: overload operator == for queries and replace with it
  ASSERT_EQ(iroha::hash(*res_query.value()), iroha::hash(*query));
}

TEST(PbQueryFactoryTest, get_roles) {
  auto query = QueryGenerator{}.generateGetRoles();
  runQueryTest(query);
}

TEST(PbQueryFactoryTest, get_role_permissions) {
  auto query = QueryGenerator{}.generateGetRolePermissions();
  runQueryTest(query);
}

TEST(PbQueryFactoryTest, get_asset_info) {
  auto query = QueryGenerator{}.generateGetAssetInfo();
  runQueryTest(query);
}
