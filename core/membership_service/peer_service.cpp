/*
Copyright Soramitsu Co., Ltd. 2016 All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language gover
ning permissions and
limitations under the License.
*/

//
// Created by Takumi Yamashita on 2017/03/16.
//

#include <algorithm>
#include <deque>
#include <regex>

//#include <connection/connection.hpp>
#include <infra/config/peer_service_with_json.hpp>
#include <membership_service/peer_service.hpp>

namespace peer {

using PeerServiceConfig = config::PeerServiceConfig;
Nodes peerList;
bool is_active;

namespace myself {

std::string getPublicKey() {
  return PeerServiceConfig::getInstance().getMyPublicKey();
}

std::string getPrivateKey() {
  return PeerServiceConfig::getInstance().getMyPrivateKey();
}

std::string getIp() { return PeerServiceConfig::getInstance().getMyIp(); }

bool isActive() { return is_active; }
void activate() { is_active = true; }
void stop() { is_active = false; }

bool isLeader() {
  auto sorted_peers = service::getActivePeerList();
  if (sorted_peers.empty()) return false;
  auto peer = *sorted_peers.begin();
  return peer->publicKey == getPublicKey() && peer->ip == getIp();
}

}  // namespace myself

namespace service {

// this function must be invoke before use peer-service.
void initialize() {
  if (!peerList.empty()) {
    return;
  }
  for (const auto &json_peer : PeerServiceConfig::getInstance().getGroup()) {
    peerList.push_back(std::make_shared<Node>(
        json_peer["ip"].get<std::string>(),
        json_peer["publicKey"].get<std::string>(),
        PeerServiceConfig::getInstance().getMaxTrustScore()));
  }
  is_active = false;
}

size_t getMaxFaulty() {
  return std::max(0, ((int)getActivePeerList().size() - 1) / 3);
}

Nodes getAllPeerList() {
  initialize();
  return peerList;
}
Nodes getActivePeerList() {
  initialize();

  Nodes nodes;
  for (const auto &node : peerList) {
    if (node->active) {
      nodes.push_back(
          std::make_unique<peer::Node>(node->ip, node->publicKey, node->trust));
    }
  }

  // TODO: maintain nodes already sorted
  sort(nodes.begin(), nodes.end(), [](const auto &a, const auto &b) {
    return a->trust > b->trust;
  });

  return nodes;
}

std::vector<std::string> getIpList() {
  std::vector<std::string> ret_ips;
  for (const auto &node : getActivePeerList()) {
    ret_ips.push_back(node->ip);
  }
  return ret_ips;
}

// is exist which peer?
bool isExistIP(const std::string &ip) {
  return findPeerIP(std::move(ip)) != peerList.end();
}

bool isExistPublicKey(const std::string &publicKey) {
  return findPeerPublicKey(std::move(publicKey)) != peerList.end();
}

Nodes::iterator findPeerIP(const std::string &ip) {
  initialize();
  return std::find_if(peerList.begin(), peerList.end(),
                      [&ip](const auto &p) { return p->ip == ip; });
}

Nodes::iterator findPeerPublicKey(const std::string &publicKey) {
  initialize();
  return std::find_if(
      peerList.begin(), peerList.end(),
      [&publicKey](const auto &p) { return p->publicKey == publicKey; });
}

std::shared_ptr<peer::Node> leaderPeer() {
  return std::move(*getActivePeerList().begin());
}

}  // namespace service

namespace transaction {

namespace isssue {
// invoke to issue transaction
void add(const peer::Node &peer) {
  if (service::isExistIP(peer.ip) || service::isExistPublicKey(peer.publicKey))
    return;
  /*  auto txPeer =
        TransactionBuilder<Add<Peer>>()
            .setSenderPublicKey(myself::getPublicKey())
            .setPeer(txbuilder::createPeer(
                peer.publicKey, peer.ip,
                txbuilder::createTrust(
                    PeerServiceConfig::getInstance().getMaxTrustScore(),
    false)))
            .build();
    connection::iroha::PeerService::Sumeragi::send(myself::getPublicKey(),
                                                   txPeer);
                                                   */
}

void remove(const std::string &publicKey) {
  if (!service::isExistPublicKey(publicKey)) return;
  /*  auto txPeer =
        TransactionBuilder<Remove<Peer>>()
            .setSenderPublicKey(myself::getPublicKey())
            .setPeer(txbuilder::createPeer(
                publicKey, peer::defaultIP(),
                txbuilder::createTrust(
                    -PeerServiceConfig::getInstance().getMaxTrustScore(),
    false)))
            .build();
    connection::iroha::PeerService::Sumeragi::send(myself::getPublicKey(),
                                                   txPeer);
                                                   */
}

void setTrust(const std::string &publicKey, const double &trust) {}
void changeTrust(const std::string &publicKey, const double &trust) {}
void setActive(const std::string &publicKey, const bool active) {}

}  // namespace isssue
namespace executor {
// invoke when execute transaction
bool add(const peer::Node &peer) {
  try {
    if (service::isExistIP(peer.ip))
      throw exception::service::DuplicationIPException(peer.ip);
    if (service::isExistPublicKey(peer.publicKey))
      throw exception::service::DuplicationPublicKeyException(peer.publicKey);
    peerList.emplace_back(std::make_shared<peer::Node>(peer));
  } catch (exception::service::DuplicationPublicKeyException &e) {
    logger::warning("addPeer") << e.what();
    return false;
  } catch (exception::service::DuplicationIPException &e) {
    logger::warning("addPeer") << e.what();
    return false;
  }
  return true;
}
bool remove(const std::string &publicKey) {
  try {
    auto it = service::findPeerPublicKey(publicKey);
    if (!service::isExistPublicKey(publicKey))
      throw exception::service::UnExistFindPeerException(publicKey);
    peerList.erase(it);
  } catch (exception::service::UnExistFindPeerException &e) {
    logger::warning("removePeer") << e.what();
    return false;
  }
  return true;
}

bool setTrust(const std::string &publicKey, const double &trust) {
  try {
    if (!service::isExistPublicKey(publicKey))
      throw exception::service::UnExistFindPeerException(publicKey);
    service::findPeerPublicKey(publicKey)->get()->trust =
        std::min(PeerServiceConfig::getInstance().getMaxTrustScore(), trust);
  } catch (exception::service::UnExistFindPeerException &e) {
    logger::warning("validate setTrust") << e.what();
    return false;
  }
  return true;
}

bool changeTrust(const std::string &publicKey, const double &trust) {
  try {
    if (!service::isExistPublicKey(publicKey))
      throw exception::service::UnExistFindPeerException(publicKey);
    auto it = service::findPeerPublicKey(publicKey)->get();
    it->trust += trust;
    it->trust = std::min(PeerServiceConfig::getInstance().getMaxTrustScore(),
                         it->trust);
  } catch (exception::service::UnExistFindPeerException &e) {
    logger::warning("validate changeTrust") << e.what();
    return false;
  }
  return true;
}

bool setActive(const std::string &publicKey, const bool active) {
  try {
    if (!service::isExistPublicKey(publicKey))
      throw exception::service::UnExistFindPeerException(publicKey);
    service::findPeerPublicKey(publicKey)->get()->active = active;
  } catch (exception::service::UnExistFindPeerException &e) {
    logger::warning("validate setActive") << e.what();
    return false;
  }
  return true;
}

}  // namespace executor

namespace validator {
// invoke when validator transaction
bool add(const peer::Node &peer) {
  try {
    if (service::isExistIP(peer.ip))
      throw exception::service::DuplicationIPException(std::move(peer.ip));
    if (service::isExistPublicKey(peer.publicKey))
      throw exception::service::DuplicationPublicKeyException(
          std::move(peer.publicKey));
  } catch (exception::service::DuplicationPublicKeyException &e) {
    logger::warning("validate addPeer") << e.what();
    return false;
  } catch (exception::service::DuplicationIPException &e) {
    logger::warning("validate addPeer") << e.what();
    return false;
  }
  return true;
}
bool remove(const std::string &publicKey) {
  try {
    if (!service::isExistPublicKey(publicKey))
      throw exception::service::UnExistFindPeerException(publicKey);
  } catch (exception::service::UnExistFindPeerException &e) {
    logger::warning("validate removePeer") << e.what();
    return false;
  }
  return true;
}

bool setTrust(const std::string &publicKey, const double &trust) {
  try {
    if (!service::isExistPublicKey(publicKey))
      throw exception::service::UnExistFindPeerException(publicKey);
  } catch (exception::service::UnExistFindPeerException &e) {
    logger::warning("validate setTrust") << e.what();
    return false;
  }
  return true;
}

bool changeTrust(const std::string &publicKey, const double &trust) {
  try {
    if (!service::isExistPublicKey(publicKey))
      throw exception::service::UnExistFindPeerException(publicKey);
  } catch (exception::service::UnExistFindPeerException &e) {
    logger::warning("validate changeTrust") << e.what();
    return false;
  }
  return true;
}

bool setActive(const std::string &publicKey, const bool active) {
  try {
    if (!service::isExistPublicKey(publicKey))
      throw exception::service::UnExistFindPeerException(publicKey);
  } catch (exception::service::UnExistFindPeerException &e) {
    logger::warning("validate setActive") << e.what();
    return false;
  }
  return true;
}

}  // namespace validator
}  // namespace transaction
}  // namespace peer
