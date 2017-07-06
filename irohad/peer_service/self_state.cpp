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
#include <peer_service/monitor.hpp>
#include <peer_service/self_state.hpp>

#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <crypto/crypto.hpp>

#include <iostream>
#include <cstring>

namespace peer_service {
  namespace self_state {

    std::string ip_;
    std::string public_key_;
    std::string private_key_;
    std::string name_;
    double trust_;

    uint64_t active_time_;
    State state_;

    void initializeMyKey() {
      if (public_key_.empty() || private_key_.empty()) {
        auto seed = iroha::create_seed();
        auto keypair = iroha::create_keypair(seed);
        public_key_ = keypair.pubkey.to_base64();
        private_key_ = keypair.privkey.to_base64();
      }
    }

    void initializeMyIp() {
      if (ip_.empty()) {
        std::string interface = "eth0";  // TODO : temporary "eth0"

        int sockfd;
        struct ifreq ifr;

        sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        ifr.ifr_addr.sa_family = AF_INET;
        strncpy(ifr.ifr_name, interface.c_str(), IFNAMSIZ - 1);
        ioctl(sockfd, SIOCGIFADDR, &ifr);
        close(sockfd);
        ip_ = inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
      }
    }

    std::string getPublicKey() {
      initializeMyKey();
      return public_key_;
    }
    std::string getPrivateKey() {
      initializeMyKey();
      return private_key_;
    }
    std::string getIp() {
      initializeMyIp();
      return ip_;
    }
    std::string getName() {
      return name_;  // TODO : initialize name
    }
    State getState() { return state_; }

    bool isLeader() {
      return monitor::getCurrentLeader()->public_key_ == public_key_;
    }

    double getTrust() {
      return 100.0;  // TODO temp
    }

    uint64_t getActiveTime() { return active_time_; }

    void setName(const std::string &name) { name_ = name; }
    void setName(std::string &&name) { name_ = name; }

    void activate() {
      state_ = ACTIVE;
      active_time_ = iroha::time::now64();
    }
    void stop() { state_ = PREPARE; }
  };
};
