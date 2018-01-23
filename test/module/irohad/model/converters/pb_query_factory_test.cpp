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

#include <gtest/gtest.h>
#include "cryptography/ed25519_sha3_impl/internal/sha3_hash.hpp"
#include "model/converters/pb_query_factory.hpp"
#include "model/generators/query_generator.hpp"

#include "model/queries/get_roles.hpp"
#include "model/queries/get_asset_info.hpp"

using namespace iroha::model::converters;
using namespace iroha::model::generators;
using namespace iroha::model;

void runQueryTest(std::shared_ptr<Query> query){
  PbQueryFactory query_factory;
  auto pb_query = query_factory.serialize(query);
  ASSERT_TRUE(pb_query.has_value());
  auto res_query = query_factory.deserialize(pb_query.value());
  ASSERT_TRUE(res_query.has_value());
  // TODO 26/09/17 grimadas: overload operator == for queries and replace with it IR-512 #goodfirstissue
  ASSERT_EQ(iroha::hash(*res_query.value()), iroha::hash(*query));
}

/**
 * @given Query which query pubkey length is invalid
 * @when deserialize query
 * @then validate the thrown exception
 */
TEST(PbQueryFactoryTest, DeserializeInvalidQueryPubkey) {
  iroha::protocol::Query proto_query;
  iroha::protocol::GetAccount proto_get_account;
  proto_query.mutable_payload()->mutable_get_account()->CopyFrom(
      proto_get_account);
  proto_query.mutable_signature()->set_pubkey(std::string(10, 1));
  proto_query.mutable_signature()->set_signature(std::string(iroha::sig_t::size(), 1));
  try {
    PbQueryFactory{}.deserialize(proto_query);
    FAIL() << "must not be passed";
  } catch (const std::invalid_argument &e) {
    ASSERT_STREQ(
        "PbQueryFactory::deserialize: 'Signature::pubkey()' has incorrect "
        "length\nExpected: 32 bytes\nActual: 10 bytes\n",
        e.what());
  } catch (...) {
    FAIL() << "std::invalid_argument must be thrown";
  }
}

/**
 * @given Query which query signature length is invalid
 * @when deserialize query
 * @then validate the thrown exception
 */
TEST(PbQueryFactoryTest, DeserializeInvalidQuerySignature) {
  iroha::protocol::Query proto_query;
  iroha::protocol::GetAccount proto_get_account;
  proto_query.mutable_payload()->mutable_get_account()->CopyFrom(
      proto_get_account);
  proto_query.mutable_signature()->set_pubkey(std::string(iroha::pubkey_t::size(), 1));
  proto_query.mutable_signature()->set_signature(std::string(10, 1));
  try {
    PbQueryFactory{}.deserialize(proto_query);
    FAIL() << "must not be passed";
  } catch (const std::invalid_argument &e) {
    ASSERT_STREQ(
        "PbQueryFactory::deserialize: 'Signature::signature()' has incorrect "
        "length\nExpected: 64 bytes\nActual: 10 bytes\n",
        e.what());
  } catch (...) {
    FAIL() << "std::invalid_argument must be thrown";
  }
}

TEST(PbQueryFactoryTest, SerializeGetAccount){
  auto created_time = 111u;
  auto creator_account_id = "creator";
  auto query_counter = 222u;
  auto account_id = "test";
  PbQueryFactory query_factory;
  QueryGenerator query_generator;
  auto query = query_generator.generateGetAccount(created_time, creator_account_id, query_counter, account_id);
  auto pb_query = query_factory.serialize(query);
  ASSERT_TRUE(pb_query.has_value());
  auto &pl = pb_query.value().payload();
  auto &pb_cast = pb_query.value().payload().get_account();
  ASSERT_EQ(pl.created_time(), created_time);
  ASSERT_EQ(pl.creator_account_id(), creator_account_id);
  ASSERT_EQ(pl.query_counter(), query_counter);
  ASSERT_EQ(pb_cast.account_id(), account_id);
  auto res_query_opt = query_factory.deserialize(pb_query.value());
  ASSERT_TRUE(res_query_opt.has_value());
  auto res_query = res_query_opt.value();
  ASSERT_EQ(res_query->created_ts, created_time);
  ASSERT_EQ(res_query->creator_account_id, creator_account_id);
  ASSERT_EQ(res_query->query_counter, query_counter);
  ASSERT_EQ(std::static_pointer_cast<GetAccount>(res_query)->account_id, account_id);
  // TODO 26/09/17 grimadas: overload operator == for queries and replace with it IR-512 #goodfirstissue
  ASSERT_EQ(iroha::hash(*res_query), iroha::hash(*query));
}

TEST(PbQueryFactoryTest, SerializeGetAccountAssets){
  PbQueryFactory query_factory;
  QueryGenerator query_generator;
  auto query = query_generator.generateGetAccountAssets(0, "123", 0, "test", "coin");
  auto pb_query = query_factory.serialize(query);
  ASSERT_TRUE(pb_query.has_value());
  auto res_query = query_factory.deserialize(pb_query.value());
  ASSERT_TRUE(res_query.has_value());
  // TODO 26/09/17 grimadas: overload operator == for queries and replace with it IR-512 #goodfirstissue
  ASSERT_EQ(iroha::hash(*res_query.value()), iroha::hash(*query));
}

/**
 * @given GetAccountDetail
 * @when Set all data
 * @then Return Protobuf Data
 */
TEST(PbQueryFactoryTest, SerializeGetAccountDetail){
  PbQueryFactory query_factory;
  QueryGenerator query_generator;
  auto query = query_generator.generateGetAccountDetail(0, "123", 0, "test", "test2", "key");
  auto pb_query = query_factory.serialize(query);
  ASSERT_TRUE(pb_query.has_value());
  auto res_query = query_factory.deserialize(pb_query.value());
  ASSERT_TRUE(res_query.has_value());
  ASSERT_EQ(iroha::hash(*res_query.value()), iroha::hash(*query));
}

TEST(PbQueryFactoryTest, SerializeGetAccountTransactions){
  PbQueryFactory query_factory;
  QueryGenerator query_generator;
  auto query = query_generator.generateGetAccountTransactions(0, "123", 0, "test");
  auto pb_query = query_factory.serialize(query);
  ASSERT_TRUE(pb_query.has_value());
  auto res_query = query_factory.deserialize(pb_query.value());
  ASSERT_TRUE(res_query.has_value());
  // TODO 26/09/17 grimadas: overload operator == for queries and replace with it IR-512 #goodfirstissue
  ASSERT_EQ(iroha::hash(*res_query.value()), iroha::hash(*query));
}

TEST(PbQueryFactoryTest, SerializeGetTransactions){
  iroha::hash256_t hash1, hash2;
  hash1[0] = 1;
  hash2[1] = 2;
  auto query = QueryGenerator{}.generateGetTransactions(0, "admin", 1, {hash1, hash2});
  runQueryTest(query);
}

/**
 * @given GetTransactions which tx_hash length is invalid
 * @when deserialize query
 * @then validate the thrown exception
 */
TEST(PbQueryFactoryTest, DeserializeInvalidGetTransactions) {
  iroha::protocol::Query proto_query;
  auto proto_gettxs = proto_query.mutable_payload()->mutable_get_transactions();
  proto_gettxs->add_tx_hashes(std::string(10, 1));
  try {
    PbQueryFactory{}.deserialize(proto_query);
    FAIL() << "must not be passed";
  } catch (const std::invalid_argument &e) {
    ASSERT_STREQ(
      "PbQueryFactory::deserialize kGetTransactions: 'tx_hash' has incorrect "
        "length\nExpected: 32 bytes\nActual: 10 bytes\n",
      e.what());
  } catch (...) {
    FAIL() << "std::invalid_argument must be thrown";
  }
}

TEST(PbQueryFactoryTest, SerializeGetSignatories){
  PbQueryFactory query_factory;
  QueryGenerator query_generator;
  auto query = query_generator.generateGetSignatories(0, "123", 0, "test");
  auto pb_query = query_factory.serialize(query);
  ASSERT_TRUE(pb_query.has_value());
  auto res_query = query_factory.deserialize(pb_query.value());
  ASSERT_TRUE(res_query.has_value());
  // TODO 26/09/17 grimadas: overload operator == for queries and replace with it IR-512 #goodfirstissue
  ASSERT_EQ(iroha::hash(*res_query.value()), iroha::hash(*query));
}

TEST(PbQueryFactoryTest, get_roles){

  auto query = QueryGenerator{}.generateGetRoles();
  runQueryTest(query);
}

TEST(PbQueryFactoryTest, get_role_permissions){
  auto query = QueryGenerator{}.generateGetRolePermissions();
  runQueryTest(query);
}

TEST(PbQueryFactoryTest, get_asset_info){
  auto query = QueryGenerator{}.generateGetAssetInfo();
  runQueryTest(query);
}
