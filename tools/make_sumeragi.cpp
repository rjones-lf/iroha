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
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>

#include <json.hpp>
#include "../core/crypto/base64.hpp"
#include "../core/crypto/signature.hpp"

#include <string>
#include <iostream>
#include <fstream>

namespace tools {

    namespace make_sumeragi {

        using std::string;
        using json = nlohmann::json;

        string interface = "eth0";
        string name;
        string filename = "";

        void parse_option(int argc, char *argv[]) {
            char c;
            while ((c = getopt(argc, argv, "o:i:n:")) != -1) {
                switch (c) {
                    case 'i':
                        interface = string(optarg);
                        break;
                    case 'n':
                        name = string(optarg);
                       std::cout<<"set peer name: "<< name <<std::endl;
                        break;
                    case 'o':
                        filename = string(optarg);
                        break;
                    default:
                        std::cerr << "usage: " << argv[0] << " -o outputFileName -n peerName  -i interface" << std::endl;
                        exit(1);
                }
            }
        }

        string getMyIp() {
            int sockfd;
            struct ifreq ifr;

            sockfd = socket(AF_INET, SOCK_DGRAM, 0);
            ifr.ifr_addr.sa_family = AF_INET;
            strncpy(ifr.ifr_name, interface.c_str(), IFNAMSIZ - 1);
            ioctl(sockfd, SIOCGIFADDR, &ifr);
            close(sockfd);
            return inet_ntoa(((struct sockaddr_in *) &ifr.ifr_addr)->sin_addr);
        }

        string getConfigStr(){
            json config;
            std::cout <<"IP: "<< getMyIp() << std::endl;
            config["ip"] = getMyIp();

            signature::KeyPair keyPair = signature::generateKeyPair();
            config["publicKey"] = base64::encode(keyPair.publicKey).c_str();
            config["privateKey"] = base64::encode(keyPair.privateKey).c_str();
            if( tools::make_sumeragi::name != "") {
                config["name"] = tools::make_sumeragi::name;
            }else{
                config["name"] = "mizuki";
            }
            std::cout << config.dump(4) << std::endl;
            return config.dump();
        }

    }
}
int main(int argc, char* argv[]) {
    tools::make_sumeragi::parse_option(argc, argv);

    std::string config = tools::make_sumeragi::getConfigStr();
    if(tools::make_sumeragi::filename != ""){
        std::cout << "============" << std::endl;
        std::cout << "output: "<< tools::make_sumeragi::filename << std::endl;
        std::ofstream ofs;
        ofs.open(tools::make_sumeragi::filename);
        ofs << config << std::endl;
        ofs.close();
    }
    return 0;
}
//

