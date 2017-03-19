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

#include "../peer_repository.hpp"
#include "common_repository.hpp"
#include <crypto/hash.hpp>
#include <repository/world_state_repository.hpp>
#include <util/logger.hpp>

namespace common = ::repository::common;

const std::string NameSpaceID = "peer repository";
const auto ValuePrefix = common::Prefix("Peer::");

namespace repository {
  namespace peer {

    namespace detail {
/********************************************************************************************
 * stringify / parse
 ********************************************************************************************/
      std::string stringifyPeer(const Api::Peer &obj) {
        std::string ret;
        obj.SerializeToString(&ret);
        return ret;
      }

      Api::Peer parsePeer(const std::string &str) {
        Api::Peer ret;
        ret.ParseFromString(str);
        return ret;
      }

      std::string createPeerUuid(const std::string &publicKey) {
        return hash::sha3_256_hex(publicKey);
      }
    }

/********************************************************************************************
 * Add<Peer>
 ********************************************************************************************/
    std::string add(const std::string &publicKey, const std::string &address,
                    const Api::Trust &trust) {

      logger::explore(NameSpaceID) << "Add<Peer> publicKey: " << publicKey
                                   << " address: " << address
                                   << " trust: " << trust.value();

      const auto uuid = detail::createPeerUuid(publicKey);

      if (!exists(uuid)) {
        const auto strPeer =
            common::stringify<Api::Peer>(txbuilder::createPeer(publicKey, address, trust), ValuePrefix);
        if (world_state_repository::add(uuid, strPeer)) {
          return uuid;
        }
      }

      return "";
    }

/********************************************************************************************
 * Update<Peer>
 ********************************************************************************************/
    bool update(const std::string &uuid, const std::string &address,
                const Api::Trust &trust) {
      if (exists(uuid)) {
        const auto rval = world_state_repository::find(uuid);
        logger::explore(NameSpaceID) << "Update<Peer> uuid: " << uuid
                                     << ", address: " << address

                                     << ", trust: " << trust.value();
        auto peer = common::parse<Api::Peer>(rval, ValuePrefix);
        *peer.mutable_address() = address;
        *peer.mutable_trust() = trust;
        const auto strPeer = common::stringify<Api::Peer>(peer, ValuePrefix);
        return world_state_repository::update(uuid, strPeer);
      }
      return false;
    }

/********************************************************************************************
 * Remove<Peer>
 ********************************************************************************************/
    bool remove(const std::string &uuid) {
      if (exists(uuid)) {
        logger::explore(NameSpaceID) << "Remove<Peer> uuid: " << uuid;
        return world_state_repository::remove(uuid);
      }
      return false;
    }

/********************************************************************************************
 * find
 ********************************************************************************************/
    Api::Peer findByUuid(const std::string &uuid) {

      logger::explore(NameSpaceID + "::findByUuid") << "";
      auto strPeer = world_state_repository::find(uuid);
      if (not strPeer.empty()) {
        return common::parse<Api::Peer>(strPeer, ValuePrefix);
      }

      return Api::Peer();
    }

    bool exists(const std::string &uuid) {
      const auto result = world_state_repository::exists(uuid);
      logger::explore(NameSpaceID + "::exists") << (result ? "true" : "false");
      return result;
    }
  }
}
