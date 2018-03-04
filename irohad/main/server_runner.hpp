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

#ifndef MAIN_SERVER_RUNNER_HPP
#define MAIN_SERVER_RUNNER_HPP

#include <grpc++/grpc++.h>
#include <grpc++/impl/codegen/service_type.h>

#include "common/result.hpp"

/**
 * Class runs Torii server for handling queries and commands.
 */
class ServerRunner {
 public:
  /**
   * Constructor. Initialize a new instance of ServerRunner class.
   * @param address - the address the server will be bind to in URI form
   * @param reuse - allow multiple sockets to bind to the same port
   */
  explicit ServerRunner(const std::string &address, bool reuse = true);

  /**
   * Adds a new grpc service to be run.
   * @param service - service to append.
   * @return reference to this with service appended
   */
  ServerRunner &append(std::shared_ptr<grpc::Service> service);

  /**
   * Initialize the server and run main loop.
   * @return Result with used port number or error message
   */
  iroha::expected::Result<int, std::string> run();

  /**
   * Wait until the server is up.
   */
  void waitForServersReady();

 private:
  std::unique_ptr<grpc::Server> serverInstance_;
  std::mutex waitForServer_;
  std::condition_variable serverInstanceCV_;

  std::string serverAddress_;
  bool reuse_;
  std::vector<std::shared_ptr<grpc::Service>> services_;
};

#endif  // MAIN_SERVER_RUNNER_HPP
