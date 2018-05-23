/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>
#include <boost/filesystem.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include "ametsuchi/impl/storage_impl.hpp"

using namespace iroha::ametsuchi;
using namespace iroha::expected;

class StorageTest : public ::testing::Test {
 protected:
  std::string block_store_path =
      (boost::filesystem::temp_directory_path() / "block_store").string();

  // generate random valid dbname
  std::string dbname_ = "d"
      + boost::uuids::to_string(boost::uuids::random_generator()())
            .substr(0, 8);

  std::string pg_opt_without_dbname_ =
      "host=localhost port=5432 user=postgres password=mysecretpassword";
  std::string pgopt_ = pg_opt_without_dbname_ + " dbname=" + dbname_;

  void TearDown() override {
    auto temp_connection =
        std::make_unique<pqxx::lazyconnection>(pg_opt_without_dbname_);
    auto nontx = std::make_unique<pqxx::nontransaction>(*temp_connection);
    nontx->exec("DROP DATABASE IF EXISTS " + dbname_);
  }
};

/**
 * @given Postgres options string with dbname param
 * @when Create storage using that options string
 * @then Database is created
 */
TEST_F(StorageTest, CreateStorageWithDatabase) {
  StorageImpl::create(block_store_path, pgopt_)
      .match([](const Value<std::shared_ptr<StorageImpl>> &) { SUCCEED(); },
             [](const Error<std::string> &error) { FAIL() << error.error; });
  auto temp_connection =
      std::make_unique<pqxx::lazyconnection>(pg_opt_without_dbname_);
  auto transaction = std::make_unique<pqxx::nontransaction>(*temp_connection);
  auto result = transaction->exec(
      "SELECT datname FROM pg_catalog.pg_database WHERE datname = "
      + transaction->quote(dbname_));
  ASSERT_EQ(result.size(), 1);
}

/**
 * @given Bad Postgres options string with nonexisting user in it
 * @when Create storage using that options string
 * @then Database is not created and error case is executed
 */
TEST_F(StorageTest, CreateStorageWithInvalidPgOpt) {
  std::string pg_opt = "host=localhost port=5432 users=nonexistinguser";
  StorageImpl::create(block_store_path, pg_opt)
      .match(
          [](const Value<std::shared_ptr<StorageImpl>> &) {
            FAIL() << "storage created, but should not";
          },
          [](const Error<std::string> &) { SUCCEED(); });
}
