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

#include <boost/process.hpp>
#include "gtest/gtest.h"

using namespace boost::process;
using namespace std::chrono_literals;

const std::string path_irohad(PATHIROHAD);
const std::string irohad_executable = path_irohad + "/irohad";
const std::string path_example = path_irohad + "/../../example";
const std::string path_config = path_example + "/config.sample";
const std::string path_genesis = path_example + "/genesis.block";
const std::string path_keypair = path_example + "/node0";
const std::string params = "--config " + path_config + " --genesis_block "
    + path_genesis + " --keypair_name " + path_keypair;
const auto timeout = 1s;

/*
 * @given path to irohad executable
 * @when run irohad without any parameters
 * @then irohad should not start
 */
TEST(IrohadTest, RunIrohadWithoutArgs) {
  child c(irohad_executable);
  c.wait_for(timeout);
  ASSERT_FALSE(c.running());
}

/*
 * @given path to irohad executable and paths to files irohad is needed to be
 * run (config, genesis block, keypair)
 * @when run irohad with all parameters it needs to operate as a full node
 * @then irohad should be started and running until timeout expired
 */
TEST(IrohadTest, RunIrohad) {
  child c(irohad_executable + " " + params);
  std::this_thread::sleep_for(timeout);
  ASSERT_TRUE(c.running());
  c.terminate();
}
