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
#include "model/generators/query_generator.hpp"
#include "model/sha3_hash.hpp"

#include "model/queries/get_asset_info.hpp"
#include "model/queries/get_roles.hpp"

using namespace iroha::model::converters;
using namespace iroha::model::generators;
using namespace iroha::model;

void runQueryTest(std::shared_ptr<Query> query) {
  PbQueryFactory query_factory;
  auto pb_query = query_factory.serialize(query);
  ASSERT_TRUE(pb_query);
  auto res_query = query_factory.deserialize(pb_query.value());
  ASSERT_TRUE(res_query);
  // TODO 26/09/17 grimadas: overload operator == for queries and replace with
  // it IR-512 #goodfirstissue
  ASSERT_EQ(iroha::hash(*(*res_query)), iroha::hash(*query));
}

TEST(PbQueryFactoryTest, SerializeGetAccount) {
  auto created_time = 111u;
  auto creator_account_id = "creator";
  auto query_counter = 222u;
  auto account_id = "test";
  PbQueryFactory query_factory;
  QueryGenerator query_generator;
  auto query = query_generator.generateGetAccount(
      created_time, creator_account_id, query_counter, account_id);
  auto pb_query = query_factory.serialize(query);
  ASSERT_TRUE(pb_query);
  auto &pl = pb_query.value().payload();
  auto &pb_cast = pb_query.value().payload().get_account();
  ASSERT_EQ(pl.created_time(), created_time);
  ASSERT_EQ(pl.creator_account_id(), creator_account_id);
  ASSERT_EQ(pl.query_counter(), query_counter);
  ASSERT_EQ(pb_cast.account_id(), account_id);
  auto res_query_opt = query_factory.deserialize(pb_query.value());
  ASSERT_TRUE(res_query_opt);
  auto res_query = *res_query_opt;
  ASSERT_EQ(res_query->created_ts, created_time);
  ASSERT_EQ(res_query->creator_account_id, creator_account_id);
  ASSERT_EQ(res_query->query_counter, query_counter);
  ASSERT_EQ(std::static_pointer_cast<GetAccount>(res_query)->account_id,
            account_id);
  // TODO 26/09/17 grimadas: overload operator == for queries and replace with
  // it IR-512 #goodfirstissue
  ASSERT_EQ(iroha::hash(*res_query), iroha::hash(*query));
}

TEST(PbQueryFactoryTest, SerializeGetAccountAssets) {
  PbQueryFactory query_factory;
  QueryGenerator query_generator;
  auto query =
      query_generator.generateGetAccountAssets(0, "123", 0, "test", "coin");
  auto pb_query = query_factory.serialize(query);
  ASSERT_TRUE(pb_query);
  auto res_query = query_factory.deserialize(*pb_query);
  ASSERT_TRUE(res_query);
  // TODO 26/09/17 grimadas: overload operator == for queries and replace with
  // it IR-512 #goodfirstissue
  ASSERT_EQ(iroha::hash(*(*res_query)), iroha::hash(*query));
}

/**
 * @given GetAccountDetail
 * @when Set all data
 * @then Return Protobuf Data
 */
TEST(PbQueryFactoryTest, SerializeGetAccountDetail) {
  PbQueryFactory query_factory;
  QueryGenerator query_generator;
  auto query =
      query_generator.generateGetAccountDetail(0, "123", 0, "test", "test2");
  auto pb_query = query_factory.serialize(query);
  ASSERT_TRUE(pb_query);
  auto res_query = query_factory.deserialize(*pb_query);
  ASSERT_TRUE(res_query);
  ASSERT_EQ(iroha::hash(*(*res_query)), iroha::hash(*query));
}

TEST(PbQueryFactoryTest, SerializeGetAccountTransactions) {
  PbQueryFactory query_factory;
  QueryGenerator query_generator;
  auto query =
      query_generator.generateGetAccountTransactions(0, "123", 0, "test");
  auto pb_query = query_factory.serialize(query);
  ASSERT_TRUE(pb_query);
  auto res_query = query_factory.deserialize(*pb_query);
  ASSERT_TRUE(res_query);
  // TODO 26/09/17 grimadas: overload operator == for queries and replace with
  // it IR-512 #goodfirstissue
  ASSERT_EQ(iroha::hash(*(*res_query)), iroha::hash(*query));
}

TEST(PbQueryFactoryTest, SerializeGetTransactions) {
  iroha::hash256_t hash1, hash2;
  hash1[0] = 1;
  hash2[1] = 2;
  auto query =
      QueryGenerator{}.generateGetTransactions(0, "admin", 1, {hash1, hash2});
  runQueryTest(query);
}

TEST(PbQueryFactoryTest, SerializeGetSignatories) {
  PbQueryFactory query_factory;
  QueryGenerator query_generator;
  auto query = query_generator.generateGetSignatories(0, "123", 0, "test");
  auto pb_query = query_factory.serialize(query);
  ASSERT_TRUE(pb_query);
  auto res_query = query_factory.deserialize(*pb_query);
  ASSERT_TRUE(res_query);
  // TODO 26/09/17 grimadas: overload operator == for queries and replace with
  // it IR-512 #goodfirstissue
  ASSERT_EQ(iroha::hash(*(*res_query)), iroha::hash(*query));
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
