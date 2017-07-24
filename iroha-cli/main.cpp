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

#include <ed25519.h>
#include <gflags/gflags.h>
#include <cstring>
#include <fstream>
#include <iostream>

#include "client.hpp"


DEFINE_bool(new_account, false, "Choose if account does not exist");
DEFINE_string(name, "", "Name of the account");


DEFINE_bool(grpc, false, "send sample transaction to IrohaNetwork");
DEFINE_string(address, "127.0.0.1", "Where is Iroha network");
DEFINE_int32(port, 50051, "What port to access iroha's Torii");

void create_account(std::string name);

int main(int argc, char* argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  gflags::ShutDownCommandLineFlags();

  if (FLAGS_new_account) {
    if (std::ifstream(FLAGS_name + ".pub")) {
      std::cout << "File already exists" << std::endl;
      return -1;
    }
    create_account(FLAGS_name);

  // Send test tx to Iroha
  } else if (FLAGS_grpc) {
    if (FLAGS_port > 0 && FLAGS_port < 65535) {
      std::cout<< "Send transaction to "<< FLAGS_address <<":" << FLAGS_port << std::endl;
      auto client = iroha_cli::CliClient(FLAGS_address,FLAGS_port);
      // ToDo more variables transaction
      client.sendTx(iroha::model::Transaction{});
    } else {
      std::cout<< "Invalid port number " << FLAGS_port << std::endl;
      iroha_cli::CliClient(FLAGS_address,FLAGS_port);
    }
  }
}

std::string hex_str(unsigned char* data, int len) {
  constexpr char hexmap[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                             '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
  std::string s((unsigned long)(len * 2), ' ');
  for (int i = 0; i < len; ++i) {
    s[2 * i] = hexmap[(data[i] & 0xF0) >> 4];
    s[2 * i + 1] = hexmap[data[i] & 0x0F];
  }
  return s;
}

/**
 * Command to create a new account using the interactive console.
 */
void create_account(std::string name) {
  unsigned char public_key[32], private_key[64], seed[32];

  ed25519_create_keypair(public_key, private_key, seed);
  auto pub_hex = hex_str(public_key, 32);

  auto priv_hex = hex_str(private_key, 64);

  // Save pubkey to file
  std::ofstream pub_file(name + ".pub");
  pub_file << pub_hex;
  pub_file.close();

  // Save privkey to file
  std::ofstream priv_file(name + ".priv");
  priv_file << priv_hex;
  priv_file.close();

  std::cout << "Public and private key has been generated in current directory"
            << std::endl;
}
