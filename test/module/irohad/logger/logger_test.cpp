/*
Copyright Soramitsu Co., Ltd. 2016 All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "logger/logger.hpp"
#include <gtest/gtest.h>

#include <vector>

TEST(LoggerTest, getLoggerTest) {
  auto one_logger = logger::log("one_logger");
  one_logger->info("one logger");

  auto another_logger = logger::log("another_logger");
  another_logger->warn("another logger");
  another_logger->info("temporal output {}, {}", 123, "string param");
  another_logger->info(logger::red("color output"));
  another_logger->info(
      logger::yellow("color args output {} // note: require char *").c_str(),
      "=^._.^=");
}

TEST(LoggerTest, boolReprTest) {
  ASSERT_EQ("true", logger::boolRepr(true));
  ASSERT_EQ("false", logger::boolRepr(false));
}

TEST(LoggerTest, logBoolTest) {
  ASSERT_EQ("true", logger::logBool(1));
  ASSERT_EQ("false", logger::boolRepr((void *)nullptr));
}

TEST(LoggerTest, collectionToStringNotEmpty) {
  std::vector<int> collection{1, 2, 3};
  auto res = logger::to_string(collection,
                               [](auto val) { return std::to_string(val); });
  ASSERT_EQ("{1, 2, 3}", res);
}

TEST(LoggerTest, collectionToStringEmpty) {
  std::vector<int> collection{};
  auto res = logger::to_string(collection,
                               [](auto val) { return std::to_string(val); });
  ASSERT_EQ("{}", res);
}
