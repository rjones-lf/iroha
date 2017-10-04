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

#include <gflags/gflags.h>
#include <grpc++/grpc++.h>
#include <fstream>
#include <thread>
#include "main/application.hpp"
#include "main/iroha_conf_loader.hpp"
#include "main/raw_block_insertion.hpp"
#include "crypto/keys_manager_impl.hpp"

#include "logger/logger.hpp"

bool validate_config(const char *flag_name, std::string const &path) {
  return not path.empty();
}

bool validate_genesis_path(const char *flag_name, std::string const &path) {
  return not path.empty();
}

bool validate_keypair_name(const char *flag_name, std::string const &path) {
  return not path.empty();
}

DEFINE_string(config, "", "Specify iroha provisioning path.");
DEFINE_validator(config, &validate_config);

DEFINE_string(genesis_block, "genesis.json", "Specify file with initial block");
DEFINE_validator(genesis_block, &validate_genesis_path);

DEFINE_string(keypair_name, "", "Specify name of .pub and .priv files");
DEFINE_validator(keypair_name, &validate_keypair_name);

int main(int argc, char *argv[]) {
  auto log = logger::log("MAIN");
  log->info("start");
  namespace mbr = config_members;

  gflags::ParseCommandLineFlags(&argc, &argv, true);
  gflags::ShutDownCommandLineFlags();

  auto config = parse_iroha_config(FLAGS_config);
  log->info("config initialized");

  iroha::KeysManagerImpl keysManager(FLAGS_keypair_name);
  iroha::keypair_t keypair{};
  if (auto loadedKeypair = keysManager.loadKeys()) {
    keypair = *loadedKeypair;
    std::string keypair_test = "1";
    auto signature = iroha::sign(keypair_test, keypair.pubkey, keypair.privkey);
    if (not iroha::verify(keypair_test, keypair.pubkey, signature)) {
      log->error("Failed to verify loaded keypair");
      return EXIT_FAILURE;
    }
  } else {
    log->error("Failed to load keypair");
    return EXIT_FAILURE;
  }

  Irohad irohad(config[mbr::BlockStorePath].GetString(),
                config[mbr::RedisHost].GetString(),
                config[mbr::RedisPort].GetUint(),
                config[mbr::PgOpt].GetString(),
                config[mbr::ToriiPort].GetUint(),
                keypair);

  if (not irohad.storage) {
    log->error("Failed to initialize storage");
    return EXIT_FAILURE;
  }

  iroha::main::BlockInserter inserter(irohad.storage);
  auto file = inserter.loadFile(FLAGS_genesis_block);
  auto block = inserter.parseBlock(file.value());
  log->info("Block is parsed");

  if (block.has_value()) {
    inserter.applyToLedger({block.value()});
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
