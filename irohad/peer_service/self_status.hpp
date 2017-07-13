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

#include <ed25519.h>
#include <memory>
#include <model/peer.hpp>
namespace peer_service {

  using Peer = iroha::model::Peer;
  class SelfStatus {
   public:
    SelfStatus();

    std::string getIp() const;
    std::string getPublicKey() const;
    std::string getPrivateKey() const;
    uint64_t getActiveTime() const;
    Peer::PeerStatus getStatus() const;
    Peer::PeerRole getRole() const;

    /**
     * When commit successes and state of self peer is UnSynced, It is called.
     * It throw to issue Peer::Activate(self) transaction.
     */
    void activate();

    /**
     * When commit fails, it is called.
     * It throw to issue Peer::Stop(Self) transaction.
     */
    void stop();

   private:
    std::unique_ptr<Peer> self_;
    iroha::ed25519::privkey_t private_key_;
  };
}  // namespace peer_service

#endif  //__IROHA_PEER_SERVICE_SELF_STATE_HPP__
