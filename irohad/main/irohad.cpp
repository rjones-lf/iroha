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

#include <gflags/gflags.h>
#include <grpc++/grpc++.h>
#include <fstream>
#include <thread>
#include "crypto/keys_manager_impl.hpp"
#include "main/application.hpp"
#include "main/iroha_conf_loader.hpp"
#include "main/raw_block_loader.hpp"

#include "logger/logger.hpp"

bool validate_config(const char *flag_name, std::string const &path) {
  return not path.empty();
}


bool validate_keypair_name(const char *flag_name, std::string const &path) {
  return not path.empty();
}

DEFINE_string(config, "", "Specify iroha provisioning path.");
DEFINE_validator(config, &validate_config);

DEFINE_string(genesis_block, "", "Specify file with initial block");

DEFINE_string(keypair_name, "", "Specify name of .pub and .priv files");
DEFINE_validator(keypair_name, &validate_keypair_name);

int main(int argc, char *argv[]) {
  auto log = logger::log("MAIN");
  log->info("start");

  if (not config_validator_registered
      or not keypair_name_validator_registered) {
    log->error("Flag validator is not registered");
    return EXIT_FAILURE;
  }

  namespace mbr = config_members;

  gflags::ParseCommandLineFlags(&argc, &argv, true);
  gflags::ShutDownCommandLineFlags();

  auto config = parse_iroha_config(FLAGS_config);
  log->info("config initialized");

  iroha::KeysManagerImpl keysManager(FLAGS_keypair_name);
  iroha::keypair_t keypair{};
  if (auto loadedKeypair = keysManager.loadKeys()) {
    keypair = *loadedKeypair;
  } else {
    log->error("Failed to load keypair");
    return EXIT_FAILURE;
  }

  Irohad irohad(config[mbr::BlockStorePath].GetString(),
                config[mbr::RedisHost].GetString(),
                config[mbr::RedisPort].GetUint(),
                config[mbr::PgOpt].GetString(),
                config[mbr::ToriiPort].GetUint(),
                config[mbr::InternalPort].GetUint(),
                config[mbr::MaxProposalSize].GetUint(),
                std::chrono::milliseconds(config[mbr::ProposalDelay].GetUint()),
                std::chrono::milliseconds(config[mbr::VoteDelay].GetUint()),
                std::chrono::milliseconds(config[mbr::LoadDelay].GetUint()),
                keypair);

  if (not irohad.storage) {
    log->error("Failed to initialize storage");
    return EXIT_FAILURE;
  }

  if (not FLAGS_genesis_block.empty()) {
    iroha::main::BlockLoader loader;
    auto file = loader.loadFile(FLAGS_genesis_block);
    auto block = loader.parseBlock(file.value());

    if (not block.has_value()) {
      log->error("Failed to parse genesis block");
      return EXIT_FAILURE;
    }

    // clear previous storage if any
    irohad.dropStorage();

    log->info("Block is parsed");

    irohad.storage->insertBlock(block.value());
    log->info("Genesis block inserted, number of transactions: {}",
              block.value().transactions.size());
  }
  // init pipeline components
  irohad.init();

  // runs iroha
  log->info("Running iroha");
  irohad.run();

  return 0;
}
