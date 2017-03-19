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

#include <consensus/connection/connection.hpp>
#include <infra/config/peer_service_with_json.hpp>
#include <repository/transaction_repository.hpp>
#include <service/peer_service.hpp>
#include <transaction_builder/transaction_builder.hpp>

namespace peer {

  using PeerServiceConfig = config::PeerServiceConfig;
  using txbuilder::TransactionBuilder;
  using type_signatures::Update;
  using type_signatures::Add;
  using type_signatures::Remove;
  using type_signatures::Peer;

  Nodes peerList;
  bool is_active;

  namespace myself {

    std::string getPublicKey() {
      return PeerServiceConfig::getInstance().getMyPublicKeyWithDefault(
          "Sht5opDIxbyK+oNuEnXUs5rLbrvVgb2GjSPfqIYGFdU=");
    }

    std::string getPrivateKey() {
      return PeerServiceConfig::getInstance().getMyPrivateKeyWithDefault(
          "aGIuSZRhnGfFyeoKNm/"
              "NbTylnAvRfMu3KumOEfyT2HPf36jSF22m2JXWrdCmKiDoshVqjFtZPX3WXaNuo9L8WA==");
    }

    std::string getIp() {
      return PeerServiceConfig::getInstance().getMyIpWithDefault("172.17.0.6");
    }

    bool isActive() { return is_active; }

    void activate() { is_active = true; }

    void stop() { is_active = false; }

    bool isLeader() {
      auto sorted_peers = service::getPeerList();
      if (sorted_peers.empty())
        return false;
      auto peer = *sorted_peers.begin();
      return peer->publicKey == getPublicKey() &&
             peer->ip == getIp();
    }

  } // namespace myself

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
    }

    size_t getMaxFaulty() {
      return std::max(0, ((int) getPeerList().size() - 1) / 3);
    }

    Nodes getPeerList() {
      initialize();

      Nodes nodes;
      for (const auto &node : peerList) {
        if (node->isok) {
          nodes.push_back(std::make_unique<peer::Node>(
              node->ip, node->publicKey, node->trustScore));
        }
      }

      // TODO: maintain nodes already sorted
      sort(nodes.begin(), nodes.end(), [](const auto &a, const auto &b) {
        return a->trustScore > b->trustScore;
      });

      return nodes;
    }

    std::vector<std::string> getIpList() {
      std::vector<std::string> ret_ips;
      for (const auto &node : getPeerList()) {
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
      return std::move(*getPeerList().begin());
    }

  } // namespace service

  namespace transaction {

// Initialize
    namespace izanami {
      void finished() {
        std::string leader_ip = service::leaderPeer()->ip;
        auto txPeer = TransactionBuilder<Update<Peer>>()
            .setSenderPublicKey(myself::getPublicKey())
            .setPeer(txbuilder::createPeer(
                myself::getPublicKey(), myself::getIp(),
                txbuilder::createTrust(0.0, true)))
            .build();
        connection::iroha::PeerService::Sumeragi::send(leader_ip, txPeer);
        myself::activate();
      }

// invoke next to addPeer
      bool start(const Node &peer) {
        logger::debug("peer-service") << "in sendAllTransactionToNewPeer";
        // when my node is not active, it don't send data.
        if (!(*service::findPeerPublicKey(myself::getPublicKey()))->isok) {
          return false;
        }

        uint64_t code = 0UL;
        { // Send PeerList data ( Reason: Can't do to construct peerList for only
          // transaction infomation. )
          logger::debug("peer-service") << "send all peer infomation";
          auto sorted_peerList = service::getPeerList();
          auto txResponse = Api::TransactionResponse();
          txResponse.set_message("Initilize send now Active PeerList info");
          txResponse.set_code(code++);
          for (auto &&peer : sorted_peerList) {
            auto txPeer =
                TransactionBuilder<Add<Peer>>()
                    .setSenderPublicKey(myself::getPublicKey())
                    .setPeer(txbuilder::createPeer(
                        peer->publicKey, peer->ip,
                        txbuilder::createTrust(
                            PeerServiceConfig::getInstance().getMaxTrustScore(),
                            true)))
                    .build();
            txResponse.add_transaction()->CopyFrom(txPeer);
          }
          if (!connection::iroha::PeerService::Izanami::send(peer.ip,
                                                             txResponse))
            return false;
        }

        if (0) { // WIP(leveldb don't active) Send transaction data separated block to
          // new peer.
          logger::debug("peer-service") << "send all transaction infomation";
          auto transactions = repository::transaction::findAll();
          std::size_t block_size = 500;
          for (std::size_t i = 0; i < transactions.size(); i += block_size) {
            auto txResponse = Api::TransactionResponse();
            txResponse.set_message("Midstream send Transactions");
            txResponse.set_code(code++);
            for (std::size_t j = i; j < i + block_size; j++) {
              txResponse.add_transaction()->CopyFrom(transactions[j]);
            }
            if (!connection::iroha::PeerService::Izanami::send(peer.ip,
                                                               txResponse))
              return false;
          }
        }

        { // end-point
          logger::debug("peer-service") << "send end-point";
          auto txResponse = Api::TransactionResponse();
          txResponse.set_message("Finished send Transactions");
          txResponse.set_code(code++);
          if (!connection::iroha::PeerService::Izanami::send(peer.ip,
                                                             txResponse))
            return false;
        }
        return true;
      }

    } // namespace izanami

    namespace isssue {
// invoke to issue transaction
      void add(const peer::Node &peer) {
        if (service::isExistIP(peer.ip) ||
            service::isExistPublicKey(peer.publicKey))
          return;
        auto txPeer =
            TransactionBuilder<Add<Peer>>()
                .setSenderPublicKey(myself::getPublicKey())
                .setPeer(txbuilder::createPeer(
                    peer.publicKey, peer.ip,
                    txbuilder::createTrust(
                        PeerServiceConfig::getInstance().getMaxTrustScore(), false)))
                .build();
        connection::iroha::PeerService::Sumeragi::send(myself::getPublicKey(),
                                                       txPeer);
      }

      void distruct(const std::string &publicKey) {
        if (!service::isExistPublicKey(publicKey))
          return;
        auto txPeer =
            TransactionBuilder<Update<Peer>>()
                .setSenderPublicKey(myself::getPublicKey())
                .setPeer(txbuilder::createPeer(publicKey, peer::defaultIP(),
                                               txbuilder::createTrust(-1.0, true)))
                .build();
        connection::iroha::PeerService::Sumeragi::send(myself::getPublicKey(),
                                                       txPeer);
      }

      void remove(const std::string &publicKey) {
        if (!service::isExistPublicKey(publicKey))
          return;
        auto txPeer =
            TransactionBuilder<Remove<Peer>>()
                .setSenderPublicKey(myself::getPublicKey())
                .setPeer(txbuilder::createPeer(
                    publicKey, peer::defaultIP(),
                    txbuilder::createTrust(
                        -PeerServiceConfig::getInstance().getMaxTrustScore(), false)))
                .build();
        connection::iroha::PeerService::Sumeragi::send(myself::getPublicKey(),
                                                       txPeer);
      }

      void credit(const std::string &publicKey) {
        if (!service::isExistPublicKey(publicKey))
          return;
        if ((*service::findPeerPublicKey(publicKey))->trustScore ==
            PeerServiceConfig::getInstance().getMaxTrustScore()) {
          return;
        }
        auto txPeer =
            TransactionBuilder<Update<Peer>>()
                .setSenderPublicKey(myself::getPublicKey())
                .setPeer(txbuilder::createPeer(publicKey, peer::defaultIP(),
                                               txbuilder::createTrust(+1.0, true)))
                .build();
        connection::iroha::PeerService::Sumeragi::send(myself::getPublicKey(),
                                                       txPeer);
      }
    } // namespace isssue
    namespace executor {
// invoke when execute transaction
      bool add(const peer::Node &peer) {
        try {
          if (service::isExistIP(peer.ip))
            throw exception::service::DuplicationIPException(peer.ip);
          if (service::isExistPublicKey(peer.publicKey))
            throw exception::service::DuplicationPublicKeyException(
                peer.publicKey);
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

      bool update(const std::string &publicKey, const peer::Node &peer) {
        try {
          auto it = service::findPeerPublicKey(publicKey);
          if (it == peerList.end())
            throw exception::service::UnExistFindPeerException(publicKey);

          auto pk = *it;
          if (!pk->isDefaultPubKey()) {
            auto upd_it = service::findPeerPublicKey(peer.publicKey);
            if (upd_it != it && upd_it != peerList.end())
              throw exception::service::DuplicationPublicKeyException(peer.publicKey);
            pk->publicKey = peer.publicKey;
          }

          if (!peer.isDefaultIP()) {
            auto upd_it = service::findPeerIP(peer.ip);
            if (upd_it != it && upd_it != peerList.end())
              throw exception::service::DuplicationIPException(peer.ip);
            pk->ip = peer.ip;
          }

          if (pk->trustScore != 0.0) {
            pk->trustScore =
                std::min(PeerServiceConfig::getInstance().getMaxTrustScore(),
                         pk->trustScore + peer.trustScore);
          }

          if (pk->isok != peer.isok) {
            pk->isok = peer.isok;
          }

        } catch (exception::service::UnExistFindPeerException &e) {
          logger::warning("updatePeer") << e.what();
          return false;
        } catch (exception::service::DuplicationPublicKeyException &e) {
          logger::warning("updatePeer") << e.what();
          return false;
        } catch (exception::service::DuplicationIPException &e) {
          logger::warning("updatePeer") << e.what();
          return false;
        }
        return true;
      }
    } // namespace executor

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

      bool update(const std::string &publicKey, const peer::Node &peer) {
        try {
          auto it = service::findPeerPublicKey(publicKey);
          if (it == peerList.end())
            throw exception::service::UnExistFindPeerException(publicKey);

          if (!(*it)->isDefaultPubKey()) {
            auto upd_it = service::findPeerPublicKey(peer.publicKey);
            if (upd_it != it && upd_it != peerList.end())
              throw exception::service::DuplicationPublicKeyException(
                  peer.publicKey);
          }

          if (!peer.isDefaultIP()) {
            auto upd_it = service::findPeerIP(peer.ip);
            if (upd_it != it && upd_it != peerList.end())
              throw exception::service::DuplicationIPException(peer.ip);
          }

        } catch (exception::service::UnExistFindPeerException &e) {
          logger::warning("updatePeer") << e.what();
          return false;
        } catch (exception::service::DuplicationPublicKeyException &e) {
          logger::warning("updatePeer") << e.what();
        } catch (exception::service::DuplicationIPException &e) {
          logger::warning("updatePeer") << e.what();
          return false;
        }
        return true;
      }

    } // namespace validator
  } // namespace transaction
} // namespace peer
