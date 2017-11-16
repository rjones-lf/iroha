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
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY =KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <fstream>
#include <iostream>

#include <gflags/gflags.h>
#include <boost/filesystem.hpp>

#include "client.hpp"
#include "common/assert_config.hpp"
#include "crypto/keys_manager_impl.hpp"
#include "grpc_response_handler.hpp"
#include "interactive/interactive_cli.hpp"
#include "logger/logger.hpp"
#include "model/converters/json_block_factory.hpp"
#include "model/converters/json_common.hpp"
#include "model/converters/json_query_factory.hpp"
#include "model/generators/block_generator.hpp"
#include "model/generators/signature_generator.hpp"
#include "model/model_crypto_provider_impl.hpp"
#include "responses.pb.h"
#include "validators.hpp"

DEFINE_string(config, "", "Trusted peer's ip addresses");
// DEFINE_validator(config, &iroha_cli::validate_config);

// DEFINE_string(genesis_block, "", "Genesis block for sending network");
// DEFINE_validator(genesis_block, &iroha_cli::validate_genesis_block);

DEFINE_bool(new_account, false, "Choose if account does not exist");
DEFINE_string(name, "", "Name of the account");
DEFINE_string(pass_phrase, "", "Name of the account");
DEFINE_string(key_path, ".", "Path to user keys");

// Sending transaction to Iroha
DEFINE_bool(grpc, false, "Send sample transaction to IrohaNetwork");
DEFINE_string(address, "0.0.0.0", "Address of the Iroha node");
DEFINE_int32(torii_port, 50051, "Port of iroha's Torii");
// DEFINE_validator(torii_port, &iroha_cli::validate_port);
DEFINE_string(json_transaction, "", "Transaction in json format");
DEFINE_string(json_query, "", "Query in json format");

// Genesis block generator:
DEFINE_bool(genesis_block,
            false,
            "Generate genesis block for new Iroha network");
DEFINE_string(peers_address, "", "File with peers address");

// Interactive
DEFINE_bool(interactive, false, "Interactive cli");

using namespace iroha::protocol;
using namespace iroha::model::generators;
using namespace iroha::model::converters;
using namespace iroha_cli::interactive;
namespace fs = boost::filesystem;

int main(int argc, char *argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  gflags::ShutDownCommandLineFlags();
  auto logger = logger::log("CLI-MAIN");
  if (FLAGS_new_account) {
    // Create new pub/priv key
    auto keysManager = iroha::KeysManagerImpl(FLAGS_name);
    if (not keysManager.createKeys(FLAGS_pass_phrase)) {
      logger->error("Keys already exist");
      return EXIT_FAILURE;
    } else {
      logger->info(
          "Public and private key has been generated in current directory");
    };
  } else if (FLAGS_grpc) {
    iroha_cli::CliClient client(FLAGS_address, FLAGS_torii_port);
    iroha_cli::GrpcResponseHandler response_handler;
    if (not FLAGS_json_transaction.empty()) {
      logger->info(
          "Send transaction to {}:{} ", FLAGS_address, FLAGS_torii_port);
      // Read from file
      std::ifstream file(FLAGS_json_transaction);
      if (not file.is_open()) {
        logger->error("Json transaction file not found");
        return EXIT_FAILURE;
      }
      std::string str((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());
      iroha::model::converters::JsonTransactionFactory serializer;
      auto doc = iroha::model::converters::stringToJson(str);
      if (not doc.has_value()) {
        logger->error("Json has wrong format.");
      }
      auto tx_opt = serializer.deserialize(doc.value());
      if (not tx_opt.has_value()) {
        logger->error("Json transaction has wrong format.");
      } else {
        response_handler.handle(client.sendTx(tx_opt.value()));
      }
    }
    if (not FLAGS_json_query.empty()) {
      logger->info("Send query to {}:{}", FLAGS_address, FLAGS_torii_port);
      std::ifstream file(FLAGS_json_query);
      if (not file.is_open()) {
        logger->error("Json query file not found");
        return EXIT_FAILURE;
      }
      std::string str((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());
      iroha::model::converters::JsonQueryFactory serializer;
      auto query_opt = serializer.deserialize(std::move(str));
      if (not query_opt.has_value()) {
        logger->error("Json has wrong format.");
      } else {
        response_handler.handle(client.sendQuery(query_opt.value()));
      }
    }

  } else if (FLAGS_genesis_block) {
    BlockGenerator generator;

    if (FLAGS_peers_address.empty()) {
      logger->error("--peers_address is empty");
      return -1;
    }

    std::ifstream file(FLAGS_peers_address);
    if (not file.is_open()) {
      logger->error("Peer address file not found");
      return EXIT_FAILURE;
    }
    std::vector<std::string> peers_address;
    std::copy(std::istream_iterator<std::string>(file),
              std::istream_iterator<std::string>(),
              std::back_inserter(peers_address));
    // Generate genesis block
    auto block = generator.generateGenesisBlock(peers_address);

    // Convert to json
    JsonBlockFactory json_factory;
    auto doc = json_factory.serialize(block);
    std::ofstream output_file("genesis.block");
    if (not output_file.is_open()) {
      logger->error("Cannot save genesis block file");
      return EXIT_FAILURE;
    }
    output_file << jsonToString(doc);
    logger->info("File saved to genesis.block");
  } else if (FLAGS_interactive) {
    if (FLAGS_name.empty()) {
      logger->error("Specify account name");
      return -1;
    }

    fs::path path(FLAGS_key_path);
    if (not fs::exists(path)) {
      logger->error("Path {} does not exist", path.string());
      return EXIT_FAILURE;
    }
    iroha::KeysManagerImpl manager((path / FLAGS_name).string());
    auto keypair = manager.loadKeys();
    if (not keypair.has_value()) {
      logger->error(
          "Cannot load specified keypair, or keypair is invalid. Path: {}, "
          "keypair name: {}",
          path.string(),
          FLAGS_name);
      return EXIT_FAILURE;
    }
    // TODO 13/09/17 grimadas: Init counters from Iroha, or read from disk? IR-334
    InteractiveCli interactiveCli(
        FLAGS_name,
        0,
        0,
        std::make_shared<iroha::model::ModelCryptoProviderImpl>(
            keypair.value()));
    interactiveCli.run();
  } else {
    assert_config::assert_fatal(false, "Invalid flags");
  }
  return 0;
}
