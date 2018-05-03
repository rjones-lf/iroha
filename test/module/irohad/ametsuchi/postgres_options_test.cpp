/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ametsuchi/impl/postgres_options.hpp"
#include <gtest/gtest.h>
#include <boost/algorithm/string.hpp>

using namespace iroha::ametsuchi;

/**
 * @given pg_opt string with param1 and param2
 * @when PostgresOptions object is created from given pg_opt string
 * @then PostgresOptions object successfully created @and it contains param1 and
 * param2
 */
TEST(PostgresOptionsTest, ParamExist) {
  std::string pg_opt_string = "param1=val1 param2=val2";
  auto pg_opt = PostgresOptions::create(pg_opt_string);
  pg_opt.match(
      [](iroha::expected::Value<PostgresOptions> options) {
        auto param1 = options.value.getOption("param1");
        ASSERT_TRUE(param1);
        ASSERT_EQ(param1.value(), "val1");

        auto param2 = options.value.getOption("param2");
        ASSERT_TRUE(param2);
        ASSERT_EQ(param2.value(), "val2");
      },
      [](auto) { FAIL() << "Creation of PostgresOptions object failed"; });
}

/**
 * @given pg_opt string without non_existing_param
 * @when PostgresOptions object is created from given pg_opt string
 * @then PostgresOptions object successfully created @and doesn't contain
 * non_existing_param in it
 */
TEST(PostgresOptionsTest, ParamNotExist) {
  std::string pg_opt_string = "param1=val1 param2=val2";
  auto pg_opt = PostgresOptions::create(pg_opt_string);
  pg_opt.match(
      [](iroha::expected::Value<PostgresOptions> options) {
        ASSERT_FALSE(options.value.getOption("non_existing_param"));
      },
      [](auto) { FAIL() << "Creation of PostgresOptions object failed"; });
}

/**
 * @given pg_opt string with dbname param
 * @when PostgresOptions object is created from given pg_opt string
 * @then PostgresOptions object successfully created @and doesn't contain
 * non_existing_param in it
 */
TEST(PostgresOptionsTest, DBNameParam) {
  std::string pg_opt_with_dbname = "param1=val1 param2=val2 dbname=iroha_db";
  auto pg_opt = PostgresOptions::create(pg_opt_with_dbname);
  pg_opt.match(
      [&pg_opt_with_dbname](iroha::expected::Value<PostgresOptions> options) {
        // check if dbname param exists
        auto dbname = options.value.getOption("dbname");
        ASSERT_TRUE(dbname);

        // check if optionsStringWithoutDbName returns pg_opt without dbname param
        auto pg_opt_without_dbname =
            boost::trim_copy(options.value.optionsStringWithoutDbName());
        ASSERT_EQ(pg_opt_without_dbname, "param1=val1 param2=val2");

        // check if optionsString returns full pg_opt
        auto pg_opt_str = boost::trim_copy(options.value.optionsString());
        ASSERT_EQ(pg_opt_str, pg_opt_with_dbname);
      },
      [](auto) { FAIL() << "Creation of PostgresOptions object failed"; });
}
