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
#include <peer_service/change_state.hpp>
#include <peer_service/monitor.hpp>
#include <peer_service/self_status.hpp>

#include <iostream>
#include <unordered_set>

namespace peer_service {

  extern Nodes peer_list_;
  extern Nodes active_peer_list_;

  namespace change_state {

    // This scope is issue transaction
    namespace transtion {
      // invoke to issue transaction
      void add(const std::string &ip, const Node &) {}
      void remove(const std::string &ip, const std::string &) {}
      void setTrust(const std::string &ip, const std::string &,
                    const double &) {}
      void changeTrust(const std::string &ip, const std::string &,
                       const double &) {}
      void setActive(const std::string &ip, const std::string &,
                     const State state) {}
    }

    // This scope is validation
    namespace validation {
      bool add(const Node &peer) {
        if (monitor::isExistIP(peer.ip_)) return false;
        if (monitor::isExistPublicKey(peer.public_key_)) return false;
        if (peer.getState() != PREPARE) return false;
        return true;
      }
      bool remove(const std::string &publicKey) {
        if (!monitor::isExistPublicKey(publicKey)) return false;

        auto it = monitor::findPeerPublicKey(publicKey);
        if (it->get()->getState() != PREPARE) return false;
        // TODO : ping to publickKey confirm

        return true;
      }
      bool setTrust(const std::string &publicKey, const double &trust) {
        if (!monitor::isExistPublicKey(publicKey)) return false;
        return true;
      }
      bool changeTrust(const std::string &publicKey, const double &trust) {
        if (!monitor::isExistPublicKey(publicKey)) return false;
        return true;
      }
      bool setActive(const std::string &publicKey, const State state) {
        if (!monitor::isExistPublicKey(publicKey)) return false;
        return true;
      }
    }

    // This scope is runtime
    namespace runtime {
      bool add(const Node &peer) {
        if (monitor::isExistIP(peer.ip_)) return false;
        if (monitor::isExistPublicKey(peer.public_key_)) return false;
        if (peer.getState() != PREPARE) return false;

        peer_list_.emplace_back(std::make_shared<Node>(peer));
        return true;
      }
      bool remove(const std::string &publicKey) {
        if (!monitor::isExistPublicKey(publicKey)) return false;
        auto it = monitor::findPeerPublicKey(publicKey);
        if (it->get()->getState() != PREPARE) peer_list_.erase(it);
        return true;
      }
      bool setTrust(const std::string &publicKey, const double &trust) {
        if (!monitor::isExistPublicKey(publicKey)) return false;

        auto node = *monitor::findPeerPublicKey(publicKey);
        node->setTrust(trust);
        if (node->getState() == ACTIVE) detail::changeActive(node);
        return true;
      }
      bool changeTrust(const std::string &publicKey, const double &trust) {
        if (!monitor::isExistPublicKey(publicKey)) return false;

        auto node = *monitor::findPeerPublicKey(publicKey);
        node->setTrust(node->getTrust() + trust);
        if (node->getState() == ACTIVE) detail::changeActive(node);
        return true;
      }
      bool setActive(const std::string &publicKey, const State state,
                     uint64_t created) {
        if (!monitor::isExistPublicKey(publicKey)) return false;

        auto node = *monitor::findPeerPublicKey(publicKey);
        if (node->getState() == PREPARE) {
          if (state == ACTIVE) {  // PRPARE -> ACTIVE
            node->setCreated(created);
            detail::insertActive(node);
          }
        } else if (node->getState() == ACTIVE) {
          if (state == PREPARE)  // ACTIVE -> PREPARE
            detail::eraseActive(publicKey);
        }
        node->setState(state);

        return true;
      }
    };

    namespace detail {
      void insertActive(const std::shared_ptr<Node> node) {
        for (auto it = active_peer_list_.begin(); it != active_peer_list_.end();
             it++) {
          if (*node > *(*it)) {
            active_peer_list_.insert(it, node);
            break;
          }
        }
        active_peer_list_.emplace_back(node);
      }

      void eraseActive(const std::string &publicKey) {
        for (auto it = active_peer_list_.begin(); it != active_peer_list_.end();
             it++) {
          if ((*it)->getPublicKey() == publicKey) {
            active_peer_list_.erase(it);
            break;
          }
        }
      }

      void changeActive(const std::shared_ptr<Node> node) {
        eraseActive(node->getPublicKey());
        insertActive(node);
      }
    }

    void initialize() {
      if (!peer_list_.empty()) return;
      // TODO Read config.json

      // At First myself only
      peer_list_.emplace_back(std::make_shared<Node>(
          self_state::getIp(), self_state::getPublicKey(),
          self_state::getName(), self_state::getTrust(),
          self_state::getActiveTime(), self_state::getState()));
    }
  }
};
