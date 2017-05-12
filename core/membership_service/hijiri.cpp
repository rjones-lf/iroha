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
//
// Created by Takumi Yamashita on 2017/03/16.
//
//#include <connection/connection.hpp>
#include <membership_service/hijiri.hpp>
#include <membership_service/peer_service.hpp>

namespace peer {
namespace hijiri {

// check are broken? peer
void check(const std::string &ip) {
    /*
  if (!service::isExistIP(ip)) return;
  auto check_peer_it = *service::findPeerIP(ip);
  if (!connection::iroha::PeerService::Sumeragi::ping(ip)) {
    if (check_peer_it->trustScore < 0.0) {
      transaction::isssue::remove(check_peer_it->publicKey);
    } else {
      transaction::isssue::distruct(check_peer_it->publicKey);
    }
  } else {
    transaction::isssue::credit(check_peer_it->publicKey);
  }
     */
}

}  // namespace hijiri
}  // namespace peer
