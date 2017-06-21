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
#ifndef __IROHA_PEER_SERVICE_SELF_STATE_HPP__
#define __IROHA_PEER_SERVICE_SELF_STATE_HPP__

#include <peer_service/peer_service.hpp>

namespace peer_service {
  namespace self_state {

    void initializeMyKey();
    void initializeMyIp();

    std::string getPublicKey();
    std::string getPrivateKey();
    std::string getIp();
    std::string getName();
    double getTrust();
    State getState();

    bool isLeader();

    uint64_t getActiveTime();

    void setName(const std::string &name);
    void setName(std::string &&name);
    void activate();
    void stop();
  };
};

#endif  //__IROHA_PEER_SERVICE_SELF_STATE_HPP__
