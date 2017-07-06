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

#ifndef __IROHA_PEER_SERVICE_PEER_SERVIEC_HPP__
#define __IROHA_PEER_SERVICE_PEER_SERVIEC_HPP__

#include <algorithm>
#include <datetime/time.hpp>
#include <memory>
#include <string>
#include <vector>
#include <cmath>

namespace peer_service {

  enum State { PREPARE, READY, ACTIVE };

  inline static const std::string defaultName() { return ""; }

  inline static const std::string defaultIP() { return ""; }

  inline static const std::string defaultPubKey() { return ""; }

  struct Node {
    std::string ip_;
    std::string public_key_;
    std::string name_;
    double trust_;
    uint64_t created_;
    State state_;

    Node(std::string ip = defaultIP(), std::string public_key = defaultPubKey(),
         std::string name = defaultName(), double trust = 100.0,
         uint64_t created_ = iroha::time::now64(), State state = PREPARE)
        : ip_(ip),
          public_key_(public_key),
          name_(name),
          trust_(trust),
          state_(state) {}

    Node(const Node& p)
        : ip_(p.ip_),
          public_key_(p.public_key_),
          name_(p.name_),
          trust_(p.trust_),
          created_(p.created_),
          state_(p.state_) {}

    void setIp(std::string ip = defaultIP()) { ip_ = ip; }
    void setPublicKey(std::string public_key = defaultPubKey()) {
      public_key_ = public_key;
    }
    void setName(std::string name = defaultName()) { name_ = name; }
    void setTrust(double trust = 100.0) { trust_ = trust; }
    void setCreated(uint64_t created) { created_ = created; }
    void setState(State state = PREPARE) { state_ = state; }

    std::string getIp() const { return ip_; }
    std::string getPublicKey() const { return public_key_; }
    std::string getName() const { return name_; }
    double getTrust() const { return trust_; }
    uint64_t getCreated() const { return created_; }
    State getState() const { return state_; }

    bool isDefaultIP() const { return ip_ == defaultIP(); }
    bool isDefaultPubKey() const { return public_key_ == defaultPubKey(); }

    bool operator>(const Node& node) const {
      return (fabs(trust_ - node.trust_) < 1e-5) ? created_ < node.created_
                                                     : trust_ > node.trust_;
    }
  };

  using Nodes = std::vector<std::shared_ptr<Node>>;

}  // namespace peer_service

#endif
