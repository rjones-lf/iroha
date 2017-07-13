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

#include <crypto/crypto.hpp>
#include <peer_service/self_status.hpp>

#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <cstring>

#include <datetime/time.hpp>

namespace peer_service {

  using Peer = iroha::model::Peer;

  SelfStatus::SelfStatus() {
    if (self_) return;
    self_ = std::make_unique<Peer>();
    {
      std::string interface = "eth0";  // TODO : temporary "eth0"

      int sockfd;
      struct ifreq ifr;

      sockfd = socket(AF_INET, SOCK_DGRAM, 0);
      ifr.ifr_addr.sa_family = AF_INET;
      strncpy(ifr.ifr_name, interface.c_str(), IFNAMSIZ - 1);
      ioctl(sockfd, SIOCGIFADDR, &ifr);
      close(sockfd);
      self_->ip = inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
    }
    {
      auto seed = iroha::create_seed();
      auto keypair = iroha::create_keypair(seed);
      self_->pubkey = keypair.pubkey;
      private_key_ = keypair.privkey;
    }
    self_->status = Peer::PeerStatus::UnSynced;
    self_->role = Peer::PeerRole::Validator;
    self_->activated_time = 0;
  }

  std::string SelfStatus::getPublicKey() const {
    return self_->pubkey.to_base64();
  }
  std::string SelfStatus::getPrivateKey() const {
    return private_key_.to_base64();
  }
  std::string SelfStatus::getIp() const { return self_->ip; }

  Peer::PeerStatus SelfStatus::getStatus() const { return self_->status; }

  Peer::PeerRole SelfStatus::getRole() const { return self_->role; }

  uint64_t SelfStatus::getActiveTime() const { return self_->activated_time; }

  void SelfStatus::activate() {
    self_->status = Peer::PeerStatus::Synced;
    self_->activated_time = iroha::time::now64();
  }

  void SelfStatus::stop() { self_->status = Peer::PeerStatus::Synced; }
}
