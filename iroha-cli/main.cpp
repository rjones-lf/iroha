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

#include <gflags/gflags.h>
#include <boost/filesystem.hpp>
#include <fstream>
#include <iostream>

#include "client.hpp"
#include "common/assert_config.hpp"
#include "crypto/keys_manager_impl.hpp"
#include "grpc_response_handler.hpp"
#include "interactive/interactive_cli.hpp"
#include "model/converters/json_block_factory.hpp"
#include "model/converters/json_query_factory.hpp"
#include "model/generators/block_generator.hpp"
#include "model/model_crypto_provider_impl.hpp"
#include "validators.hpp"

// Account information
DEFINE_bool(
    new_account,
    false,
    "Generate and save locally new public/private keys");
DEFINE_string(account_name,
              "",
              "Name of the account. Must be unique in iroha network");
DEFINE_string(pass_phrase, "", "Account pass-phrase");
DEFINE_string(key_path, ".", "Path to user keys");

// Iroha peer to connect with
DEFINE_string(peer_ip, "0.0.0.0", "Address of the Iroha node");
DEFINE_int32(torii_port, 50051, "Port of Iroha's Torii");

// Send already signed and formed transaction to Iroha peer
DEFINE_string(json_transaction, "", "Transaction in json format");
// Send already signed and formed query to Iroha peer
DEFINE_string(json_query, "", "Query in json format");

// Genesis block generator:
DEFINE_bool(genesis_block,
            false,
            "Generate genesis block for new Iroha network");
DEFINE_string(peers_address,
              "",
              "File with peers address for new Iroha network");

// Run iroha-cli in interactive mode
DEFINE_bool(interactive, true, "Run iroha-cli in interactive mode");


using namespace iroha::protocol;
using namespace iroha::model::generators;
using namespace iroha::model::converters;
using namespace iroha_cli::interactive;
namespace fs = boost::filesystem;

int main(int argc, char *argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  gflags::ShutDownCommandLineFlags();
  auto logger = logger::log("CLI-MAIN");
  // Generate new genesis block now Iroha network
  if (FLAGS_genesis_block) {
    BlockGenerator generator;

    if (FLAGS_peers_address.empty()) {
      logger->error("--peers_address is empty");
      return EXIT_FAILURE;
    }
    std::ifstream file(FLAGS_peers_address);
    std::vector<std::string> peers_address;
    std::copy(std::istream_iterator<std::string>(file),
              std::istream_iterator<std::string>(),
              std::back_inserter(peers_address));
    // Generate genesis block
    auto transaction = TransactionGenerator().generateGenesisTransaction(
        0, std::move(peers_address));
    auto block = generator.generateGenesisBlock(0, {transaction});
    // Convert to json
    JsonBlockFactory json_factory;
    auto doc = json_factory.serialize(block);
    std::ofstream output_file("genesis.block");
    output_file << jsonToString(doc);
    logger->info("File saved to genesis.block");
  }
  // Create new pub/priv key, register in Iroha Network
  else if (FLAGS_new_account) {
    auto keysManager = iroha::KeysManagerImpl(FLAGS_account_name);
    if (not(FLAGS_pass_phrase.size() == 0
                ? keysManager.createKeys()
                : keysManager.createKeys(FLAGS_pass_phrase))) {
      logger->error("Keys already exist");
    } else {
      logger->info(
          "Public and private key has been generated in current directory");
    }
  }
  // Send to Iroha Peer json transaction/query
  else if (not FLAGS_json_transaction.empty() or not FLAGS_json_query.empty()) {
    iroha_cli::CliClient client(FLAGS_peer_ip, FLAGS_torii_port);
    iroha_cli::GrpcResponseHandler response_handler;
    if (not FLAGS_json_transaction.empty()) {
      logger->info(
          "Send transaction to {}:{} ", FLAGS_peer_ip, FLAGS_torii_port);
      // Read from file
      std::ifstream file(FLAGS_json_transaction);
      std::string str((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());
      iroha::model::converters::JsonTransactionFactory serializer;
      auto doc = iroha::model::converters::stringToJson(str);
      if (not doc) {
        logger->error("Json has wrong format.");
      }
      auto tx_opt = serializer.deserialize(doc.value());
      if (not tx_opt) {
        logger->error("Json transaction has wrong format.");
      } else {
        response_handler.handle(client.sendTx(tx_opt.value()));
      }
    }
    if (not FLAGS_json_query.empty()) {
      logger->info("Send query to {}:{}", FLAGS_peer_ip, FLAGS_torii_port);
      std::ifstream file(FLAGS_json_query);
      std::string str((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());
      iroha::model::converters::JsonQueryFactory serializer;
      auto query_opt = serializer.deserialize(std::move(str));
      if (not query_opt) {
        logger->error("Json has wrong format.");
      } else {
        response_handler.handle(client.sendQuery(query_opt.value()));
      }
    }
  }
  // Run iroha-cli in interactive mode
  else if (FLAGS_interactive) {
    if (FLAGS_account_name.empty()) {
      logger->error("Specify your account name");
      return EXIT_FAILURE;
    }
    fs::path path(FLAGS_key_path);
    if (not fs::exists(path)) {
      logger->error("Path {} not found.", path.string());
      return EXIT_FAILURE;
    }
    iroha::KeysManagerImpl manager((path / FLAGS_account_name).string());
    boost::optional<iroha::keypair_t> keypair;
    if (FLAGS_pass_phrase.size() != 0) {
      keypair = manager.loadKeys(FLAGS_pass_phrase);
    } else {
      keypair = manager.loadKeys();
    }
    if (not keypair) {
      logger->error(
          "Cannot load specified keypair, or keypair is invalid. Path: {}, "
          "keypair name: {}. Use --key_path to path to your keypair. \nMaybe wrong pass phrase (\"{}\")?",
          path.string(),
          FLAGS_account_name,
          FLAGS_pass_phrase
      );
      return EXIT_FAILURE;
    }
    // TODO 13/09/17 grimadas: Init counters from Iroha, or read from disk?
    // IR-334
    InteractiveCli interactiveCli(
        FLAGS_account_name,
        FLAGS_peer_ip,
        FLAGS_torii_port,
        0,
        0,
        std::make_shared<iroha::model::ModelCryptoProviderImpl>(
            *keypair));
    interactiveCli.run();
  } else {
    logger->error("Invalid flags");
    return EXIT_FAILURE;
  }
  return 0;
}
