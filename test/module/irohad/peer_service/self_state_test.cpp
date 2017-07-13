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

#include <gtest/gtest.h>
#include <logger/logger.hpp>
#include <peer_service/self_status.hpp>

TEST(PeerServiceSelfState, CheckStatus) {
  static auto log = logger::Logger("PeerServiceSelfState");

  log.info("        ip: " + peer_service::self_state::getIp());
  log.info("    pubkey: " + peer_service::self_state::getPublicKey());
  log.info("privatekey: " + peer_service::self_state::getPrivateKey());
  log.info("      name: " + peer_service::self_state::getName());
  log.info("     state: " +
           std::to_string(peer_service::self_state::getState()));
  log.info("     trust: " +
           std::to_string(peer_service::self_state::getTrust()));
  log.info("activetime: " +
           std::to_string(peer_service::self_state::getActiveTime()));
}

TEST(PeerServiceSelfState, ActivateStatus) {
  static auto log = logger::Logger("PeerServiceActivateStatus");

  peer_service::self_state::activate();

  log.info("        ip: " + peer_service::self_state::getIp());
  log.info("    pubkey: " + peer_service::self_state::getPublicKey());
  log.info("privatekey: " + peer_service::self_state::getPrivateKey());
  log.info("      name: " + peer_service::self_state::getName());
  log.info("     state: " +
           std::to_string(peer_service::self_state::getState()));
  log.info("     trust: " +
           std::to_string(peer_service::self_state::getTrust()));
  log.info("activetime: " +
           std::to_string(peer_service::self_state::getActiveTime()));
}