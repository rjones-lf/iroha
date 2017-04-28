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
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <crypto/base64.hpp>
#include <crypto/signature.hpp>
#include <json.hpp>

#include <fstream>

namespace tools {
namespace make_sumeragi {

std::string interface = "eth0";
std::string name = "default";
std::string filename;

void parse_option(int argc, char *argv[]) {
  char c;
  while ((c = getopt(argc, argv, "o:i:n:h")) != -1) {
    switch (c) {
      case 'i':
        interface = optarg;
        break;
      case 'n':
        name = optarg;
        break;
      case 'o':
        filename = optarg;
        break;
      case 'h':
      default:
        std::cout << "Usage: " << argv[0] << " "
                  << "-o outputFileName "
                  << "-n peerName "
                  << "-i interface" << std::endl;
        exit(1);
    }
  }
}

std::string getMyIp() {
  int sockfd;
  struct ifreq ifr;

  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  ifr.ifr_addr.sa_family = AF_INET;
  strncpy(ifr.ifr_name, interface.c_str(), IFNAMSIZ - 1);
  ioctl(sockfd, SIOCGIFADDR, &ifr);
  close(sockfd);
  return inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
}

std::string getConfigStr() {
  nlohmann::json config;
  config["group"][0]["ip"] = config["me"]["ip"] = getMyIp();
  config["group"][0]["name"] = config["me"]["name"] = name;

  signature::KeyPair keyPair = signature::generateKeyPair();
  config["group"][0]["publicKey"] = config["me"]["publicKey"] =
      base64::encode(keyPair.publicKey);
  config["me"]["privateKey"] = base64::encode(keyPair.privateKey);

  std::cout << config.dump(2) << std::endl;
  return config.dump();
}
}
}

int main(int argc, char *argv[]) {
  tools::make_sumeragi::parse_option(argc, argv);

  std::string config = tools::make_sumeragi::getConfigStr();
  if (tools::make_sumeragi::filename != "") {
    std::cout << "============" << std::endl;
    std::cout << "output: " << tools::make_sumeragi::filename << std::endl;
    std::ofstream ofs;
    ofs.open(tools::make_sumeragi::filename);
    ofs << config << std::endl;
    ofs.close();
  }
  return 0;
}
//
