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
#include <csignal>
#include <fstream>
#include <thread>
#include "backend/protobuf/from_old_model.hpp"
#include "common/result.hpp"
#include "crypto/keys_manager_impl.hpp"
#include "main/application.hpp"
#include "main/iroha_conf_loader.hpp"
#include "main/raw_block_loader.hpp"

/**
 * Gflag validator.
 * Validator for the configuration file path input argument.
 * Path is considered to be valid if it is not empty.
 * @param flag_name - flag name. Must be 'config' in this case
 * @param path      - file name. Should be path to the config file
 * @return true if argument is valid
 */
bool validate_config(const char *flag_name, std::string const &path) {
  return not path.empty();
}

/**
 * Gflag validator.
 * Validator for the keypair files path input argument.
 * Path is considered to be valid if it is not empty.
 * @param flag_name - flag name. Must be 'keypair_name' in this case
 * @param path      - file name. Should be path to the keypair files
 * @return true if argument is valid
 */
bool validate_keypair_name(const char *flag_name, std::string const &path) {
  return not path.empty();
}

/**
 * Creating input argument for the configuration file location.
 */
DEFINE_string(config, "", "Specify iroha provisioning path.");
/**
 * Registering validator for the configuration file location.
 */
DEFINE_validator(config, &validate_config);

/**
 * Creating input argument for the genesis block file location.
 */
DEFINE_string(genesis_block, "", "Specify file with initial block");

/**
 * Creating input argument for the keypair files location.
 */
DEFINE_string(keypair_name, "", "Specify name of .pub and .priv files");
/**
 * Registering validator for the keypair files location.
 */
DEFINE_validator(keypair_name, &validate_keypair_name);

std::promise<void> exit_requested;

int main(int argc, char *argv[]) {
  auto log = logger::log("MAIN");
  log->info("start");

  // Check if validators are registered.
  if (not config_validator_registered
      or not keypair_name_validator_registered) {
    // Abort execution if not
    log->error("Flag validator is not registered");
    return EXIT_FAILURE;
  }

  namespace mbr = config_members;

  // Parsing command line arguments
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  gflags::ShutDownCommandLineFlags();

  // Reading iroha configuration file
  auto config = parse_iroha_config(FLAGS_config);
  log->info("config initialized");

  // Reading public and private key files
  iroha::KeysManagerImpl keysManager(FLAGS_keypair_name);
  iroha::keypair_t keypair{};
  // Check if both keys are read properly
  if (auto loadedKeypair = keysManager.loadKeys()) {
    keypair = *loadedKeypair;
  } else {
    // Abort execution if not
    log->error("Failed to load keypair");
    return EXIT_FAILURE;
  }

  // Configuring iroha daemon
  Irohad irohad(config[mbr::BlockStorePath].GetString(),
                config[mbr::PgOpt].GetString(),
                config[mbr::ToriiPort].GetUint(),
                config[mbr::InternalPort].GetUint(),
                config[mbr::MaxProposalSize].GetUint(),
                std::chrono::milliseconds(config[mbr::ProposalDelay].GetUint()),
                std::chrono::milliseconds(config[mbr::VoteDelay].GetUint()),
                std::chrono::milliseconds(config[mbr::LoadDelay].GetUint()),
                keypair);

  // Check if iroha daemon storage was successfully initialized
  if (not irohad.storage) {
    // Abort execution if not
    log->error("Failed to initialize storage");
    return EXIT_FAILURE;
  }

  // Check if genesis block path was specified
  if (not FLAGS_genesis_block.empty()) {
    // If it is so, read genesis block and store it to iroha storage
    iroha::main::BlockLoader loader;
    auto file = loader.loadFile(FLAGS_genesis_block);
    auto block = loader.parseBlock(file.value());

    // Check that provided genesis block file was correct
    if (not block) {
      // Abort execution if not
      log->error("Failed to parse genesis block");
      return EXIT_FAILURE;
    }

    // clear previous storage if any
    irohad.dropStorage();

    // reset ordering service persistent counter
    irohad.resetOrderingService();

    log->info("Block is parsed");

    // Applying transactions from genesis block to iroha storage
    irohad.storage->insertBlock(shared_model::proto::from_old(block.value()));
    log->info("Genesis block inserted, number of transactions: {}",
              block.value().transactions.size());
  }

  // init pipeline components
  irohad.init();

  auto handler = [](int s) { exit_requested.set_value(); };
  std::signal(SIGINT, handler);
  std::signal(SIGTERM, handler);
  std::signal(SIGQUIT, handler);

  // runs iroha
  log->info("Running iroha");
  irohad.run();
  exit_requested.get_future().wait();

  // We do not care about shutting down grpc servers
  // They do all necessary work in their destructors
  log->info("shutting down...");

  return 0;
}
