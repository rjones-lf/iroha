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
#include "common/result.hpp"
#include "crypto/keys_manager_impl.hpp"
#include "main/application.hpp"
#include "main/iroha_conf_loader.hpp"
#include "main/raw_block_loader.hpp"

static const std::string kListenIp = "0.0.0.0";

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

/**
 * Creating boolean flag for overwriting already existing block storage
 */
DEFINE_bool(overwrite_ledger, false, "Overwrite ledger data if existing");

static bool validateVerbosity(const char *flagname, int32_t val) {
  if (val >= 0 && val <= 6)
    return true;

  std::cout << "Invalid value for " << flagname << ": should be in range [0, 6]"
            << std::endl;
  return false;
}

/// Verbosity flag for spdlog configuration
DEFINE_int32(verbosity, spdlog::level::info, "Log verbosity");
DEFINE_validator(verbosity, validateVerbosity);

std::promise<void> exit_requested;

int main(int argc, char *argv[]) {
  // Parsing command line arguments
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  spdlog::set_level(spdlog::level::level_enum(FLAGS_verbosity));

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

  // Reading iroha configuration file
  auto config = parse_iroha_config(FLAGS_config);
  log->info("config initialized");

  // Reading public and private key files
  iroha::KeysManagerImpl keysManager(FLAGS_keypair_name);
  auto keypair = keysManager.loadKeys();
  // Check if both keys are read properly
  if (not keypair) {
    // Abort execution if not
    log->error("Failed to load keypair");
    return EXIT_FAILURE;
  }

  // Configuring iroha daemon
  Irohad irohad(config[mbr::BlockStorePath].GetString(),
                config[mbr::PgOpt].GetString(),
                kListenIp,  // TODO(mboldyrev) 17/10/2018: add a parameter in
                            // config file and/or command-line arguments?
                config[mbr::ToriiPort].GetUint(),
                config[mbr::InternalPort].GetUint(),
                config[mbr::MaxProposalSize].GetUint(),
                std::chrono::milliseconds(config[mbr::ProposalDelay].GetUint()),
                std::chrono::milliseconds(config[mbr::VoteDelay].GetUint()),
                *keypair,
                boost::make_optional(config[mbr::MstSupport].GetBool(),
                                     iroha::GossipPropagationStrategyParams{}));

  // Check if iroha daemon storage was successfully initialized
  if (not irohad.storage) {
    // Abort execution if not
    log->error("Failed to initialize storage");
    return EXIT_FAILURE;
  }

  /*
   * The logic implemented below is reflected in the following truth table.
   *
  +------------+--------------+------------------+---------------+---------+
  | Blockstore | New genesis  | Overwrite ledger | Genesis block | Message |
  | presence   | block is set | flag is set      | that is used  |         |
  +------------+--------------+------------------+---------------+---------+
  | 0          | 1            | 0                | new           |         |
  | 0          | 1            | 1                | new           | warning |
  | 1          | 1            | 0                | old           | warning |
  | 1          | 1            | 1                | new           |         |
  | 0          | 0            | 0                | none          | error   |
  | 0          | 0            | 1                | none          | error   |
  | 1          | 0            | 0                | old           |         |
  | 1          | 0            | 1                | old           | warning |
  +------------+--------------+------------------+---------------+---------+
   */

  /// if there are any blocks in blockstore, then true
  bool blockstore = irohad.storage->getBlockQuery()->getTopBlockHeight() != 0;

  /// genesis block file is specified as launch parameter
  bool genesis = not FLAGS_genesis_block.empty();

  /// overwrite ledger flag was set as launch parameter
  bool overwrite = FLAGS_overwrite_ledger;

  if (genesis) {  // genesis block file is specified
    if (blockstore and not overwrite) {
      log->warn(
          "Passed genesis block will be ignored without --overwrite_ledger "
          "flag. Restoring existing state.");
    } else {
      iroha::main::BlockLoader loader;
      auto file = loader.loadFile(FLAGS_genesis_block);
      auto block = loader.parseBlock(file.value());

      if (not block) {
        log->error("Failed to parse genesis block.");
        return EXIT_FAILURE;
      }

      if (not blockstore and overwrite) {
        log->warn(
            "Blockstore is empty - there is nothing to overwrite. Inserting "
            "new genesis block.");
      }

      // clear previous storage if any
      irohad.dropStorage();

      irohad.storage->insertBlock(*block.value());
      log->info("Genesis block inserted, number of transactions: {}",
                block.value()->transactions().size());
    }
  } else {  // genesis block file is not specified
    if (not blockstore) {
      log->error(
          "Cannot restore nor create new state. Blockstore is empty. No "
          "genesis block is provided. Please pecify new genesis block using "
          "--genesis_block parameter.");
      return EXIT_FAILURE;
    } else {
      if (overwrite) {
        log->warn(
            "No new genesis block is specified - blockstore cannot be "
            "overwritten. If you want overwrite ledger state, please "
            "specify new genesis block using --genesis_block parameter.");
      }
    }
  }

  // check if at least one block is available in the ledger
  auto blocks_exist = irohad.storage->getBlockQuery()->getTopBlock().match(
      [](const auto &) { return true; },
      [](iroha::expected::Error<std::string> &) { return false; });

  if (not blocks_exist) {  // may happen only in case of bug or zero disk space
    log->error(
        "You should have never seen this message. There are no blocks in the "
        "ledger.  Unable to start. Try to specify --genesis_block and "
        "--overwrite_ledger parameters at the same time.");
    return EXIT_FAILURE;
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

  gflags::ShutDownCommandLineFlags();

  return 0;
}
