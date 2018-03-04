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

#include <gtest/gtest.h>
#include <boost/format.hpp>

#include "endpoint.grpc.pb.h"  // any gRPC service is required for test
#include "main/server_runner.hpp"

boost::format address{"0.0.0.0:%d"};
auto port_visitor = iroha::make_visitor(
    [](iroha::expected::Value<int> x) { return x.value; },
    [](iroha::expected::Error<std::string> x) { return 0; });

/**
 * @given a running ServerRunner
 * @when another ServerRunner tries to run on the same port without port reuse
 * @then Result with error is returned
 */
TEST(ServerRunnerTest, SamePortNoReuse) {
  ServerRunner first_runner((address % 0).str());
  auto first_query_service =
      std::make_shared<iroha::protocol::QueryService::Service>();
  auto result = first_runner.append(first_query_service).run();
  auto port = boost::apply_visitor(port_visitor, result);
  ASSERT_NE(0, port);

  ServerRunner second_runner((address % port).str(), false);
  auto second_query_service =
      std::make_shared<iroha::protocol::QueryService::Service>();
  result = second_runner.append(second_query_service).run();
  port = boost::apply_visitor(port_visitor, result);
  ASSERT_EQ(0, port);
}

/**
 * @given a running ServerRunner
 * @when another ServerRunner tries to run on the same port with port reuse
 * @then Result with port number is returned
 */
TEST(ServerRunnerTest, SamePortWithReuse) {
  ServerRunner first_runner((address % 0).str());
  auto first_query_service =
      std::make_shared<iroha::protocol::QueryService::Service>();
  auto result = first_runner.append(first_query_service).run();
  auto port = boost::apply_visitor(port_visitor, result);
  ASSERT_NE(0, port);

  ServerRunner second_runner((address % port).str(), true);
  auto second_query_service =
      std::make_shared<iroha::protocol::QueryService::Service>();
  result = second_runner.append(second_query_service).run();
  port = boost::apply_visitor(port_visitor, result);
  ASSERT_NE(0, port);
}
