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

#include <deque>
#include <regex>
#include <algorithm>
#include <crypto/base64.hpp>
#include <util/logger.hpp>
#include <util/exception.hpp>
#include <consensus/connection/connection.hpp>
#include <json.hpp>
#include <infra/protobuf/api.pb.h>
#include <transaction_builder/transaction_builder.hpp>
#include <repository/transaction_repository.hpp>
#include "peer_service_with_json.hpp"
#include "config_format.hpp"

using PeerServiceConfig = config::PeerServiceConfig;
using txbuilder::TransactionBuilder;
using type_signatures::Update;
using type_signatures::Add;
using type_signatures::Remove;
using type_signatures::Peer;
using nlohmann::json;

std::vector<peer::Node> PeerServiceConfig::peerList;

PeerServiceConfig::PeerServiceConfig() {
    is_active = true;
    initialziePeerList_from_json();
}

PeerServiceConfig& PeerServiceConfig::getInstance() {
  static PeerServiceConfig serviceConfig;
  return serviceConfig;
}

// TODO: I could not use getParam method because of sumeragi.json is hierarchical.
// TODO: Which config file these parameter should be set? It's not sumeragi but hijiri.

std::string PeerServiceConfig::getMyPublicKeyWithDefault(const std::string& defaultValue) {
  auto config = this->getConfigData();
  if (!config.is_null()) {
    return this->getConfigData()["me"].value("publicKey", defaultValue);
  }
  return defaultValue;
}

std::string PeerServiceConfig::getMyPrivateKeyWithDefault(const std::string& defaultValue) {
  auto config = this->getConfigData();
  if (!config.is_null()) {
    return this->getConfigData()["me"].value("privateKey", defaultValue);
  }
  return defaultValue;
}

std::string PeerServiceConfig::getMyIpWithDefault(const std::string& defaultValue) {
  auto config = this->getConfigData();
  if (!config.is_null()) {
    return this->getConfigData()["me"].value("ip", defaultValue);
  }
  return defaultValue;
}

double PeerServiceConfig::getMaxTrustScoreWithDefault(double defaultValue) {
  return this->getConfigData().value("max_trust_score", defaultValue);
}

void PeerServiceConfig::parseConfigDataFromString(std::string&& jsonStr) {
  try {
    if (!ConfigFormat::getInstance().ensureFormatSumeragi(jsonStr)) {
      throw exception::ParseFromStringException(getConfigName());
    }
    _configData = json::parse(std::move(jsonStr));
  } catch (exception::ParseFromStringException& e) {
    logger::warning("peer service config") << e.what();
    logger::warning("peer service config") << getConfigName() << " is set to be default.";
  }
}

std::string PeerServiceConfig::getConfigName() {
  return "config/sumeragi.json";
}

/*
  TODO: For ease of moving peer service to another class or namespace,
      peer service config is tempolary separeted from below.
*/

std::string PeerServiceConfig::getMyPublicKey() {
  return this->getMyPublicKeyWithDefault("Sht5opDIxbyK+oNuEnXUs5rLbrvVgb2GjSPfqIYGFdU=");
}

std::string PeerServiceConfig::getMyPrivateKey() {
  return this->getMyPrivateKeyWithDefault("aGIuSZRhnGfFyeoKNm/NbTylnAvRfMu3KumOEfyT2HPf36jSF22m2JXWrdCmKiDoshVqjFtZPX3WXaNuo9L8WA==");
}

std::string PeerServiceConfig::getMyIp() {
  return this->getMyIpWithDefault("172.17.0.6");
}

double PeerServiceConfig::getMaxTrustScore() {
  return this->getMaxTrustScoreWithDefault(10.0); // WIP to support trustRate = 10.0
}

size_t PeerServiceConfig::getMaxFaulty() {
  return std::max( 0, ((int)this->getPeerList().size()-1) / 3 );
}

bool PeerServiceConfig::isMyActive() {
    return is_active;
}

void PeerServiceConfig::activate() {
    is_active = true;
}

void PeerServiceConfig::stop(){
    is_active = false;
}


// TODO: this is temporary solution
std::vector<json> PeerServiceConfig::getGroup() {
  auto config = this->getConfigData();
  if (!config.is_null()) {
     return getConfigData()["group"].get<std::vector<json>>();
  }
  return std::vector<json>({
      json({
        {"ip","172.17.0.3"},
        {"name","mizuki"},
        {"publicKey","jDQTiJ1dnTSdGH+yuOaPPZIepUj1Xt3hYOvLQTME3V0="}
      }),
      json({
        {"ip","172.17.0.4"},
        {"name","natori"},
        {"publicKey","Q5PaQEBPQLALfzYmZyz9P4LmCNfgM5MdN1fOuesw3HY="}
      }),
      json({
        {"ip","172.17.0.5"},
        {"name","kabohara"},
        {"publicKey","f5MWZUZK9Ga8XywDia68pH1HLY/Ts0TWBHsxiFDR0ig="}
      }),
      json({
        {"ip","172.17.0.6"},
        {"name","samari"},
        {"publicKey","Sht5opDIxbyK+oNuEnXUs5rLbrvVgb2GjSPfqIYGFdU="}
      })
    });
}

void PeerServiceConfig::initialziePeerList_from_json(){
  if (!peerList.empty()) return;
  for (const auto& peer : getGroup()) {
    peerList.emplace_back( peer["ip"].get<std::string>(),
                           peer["publicKey"].get<std::string>(),
                           getMaxTrustScore() );
  }
}

bool PeerServiceConfig::isExistIP( const std::string &ip ) {
  return findPeerIP( std::move(ip) ) != peerList.end();
}
bool PeerServiceConfig::isExistPublicKey( const std::string &publicKey ) {
  return findPeerPublicKey( std::move(publicKey) ) != peerList.end();
}

std::vector<peer::Node>::iterator PeerServiceConfig::findPeerIP( const std::string &ip ) {
  return std::find_if( peerList.begin(), peerList.end(),
                       [&ip]( const peer::Node& p ) { return p.getIP() == ip; } );
}

std::vector<peer::Node>::iterator PeerServiceConfig::findPeerPublicKey( const std::string &publicKey ) {
  return std::find_if( peerList.begin(), peerList.end(),
                       [&publicKey]( const peer::Node& p ) { return p.getPublicKey() == publicKey; } );
}


std::vector<std::unique_ptr<peer::Node>> PeerServiceConfig::getPeerList() {
  initialziePeerList_from_json();

  std::vector<std::unique_ptr<peer::Node>> nodes;
  for(const auto &node : peerList) {
    if(node.isOK()) {
        nodes.push_back(std::make_unique<peer::Node>(node.getIP(),
                                                     node.getPublicKey(),
                                                     node.getTrustScore()));
    }
  }

  sort(nodes.begin(), nodes.end(), [](const auto &a, const auto &b) {
    return a->getTrustScore() > b->getTrustScore();
  } );
  logger::debug("getPeerList") << std::to_string(nodes.size());
  for(const auto &node : nodes ) {
      logger::debug("getPeerList") << ("ip: " + node->getIP());
      logger::debug("getPeerList") << ("pubkey: " + node->getPublicKey() );
  }
  return nodes;
}

std::vector<std::string> PeerServiceConfig::getIpList() {
  std::vector<std::string> ret_ips;
  for(const auto &node : getPeerList() )
    ret_ips.push_back( node->getIP() );
  return ret_ips;
}


// check are broken? peer
void PeerServiceConfig::checkBrokenPeer( const std::string& ip ) {
    if (!isExistIP(ip)) return;
    auto check_peer_it = findPeerIP( ip );
    if ( !connection::iroha::PeerService::Sumeragi::ping( ip )) {
        if( check_peer_it->getTrustScore() < 0.0 ) {
            toIssue_removePeer( check_peer_it->getPublicKey() );
        } else {
            toIssue_distructPeer( check_peer_it->getPublicKey() );
        }
    } else {
        toIssue_creditPeer( check_peer_it->getPublicKey() );
    }
}

void PeerServiceConfig::finishedInitializePeer() {
    std::string leader_ip = leaderPeer()->getIP();
    auto txPeer = TransactionBuilder<Update<Peer>>()
            .setSenderPublicKey(getMyPublicKey())
            .setPeer(txbuilder::createPeer( getMyPublicKey(), getMyIp(), txbuilder::createTrust(0.0, true)))
            .build();
    connection::iroha::PeerService::Sumeragi::send( leader_ip, txPeer );
    activate();
}

// invoke to issue transaction
void PeerServiceConfig::toIssue_addPeer( const peer::Node& peer ) {
    if( isExistIP(peer.getIP()) || isExistPublicKey(peer.getPublicKey()) ) return;
    auto txPeer = TransactionBuilder<Add<Peer>>()
            .setSenderPublicKey(getMyPublicKey())
            .setPeer( txbuilder::createPeer( peer.getPublicKey(), peer.getIP(), txbuilder::createTrust(getMaxTrustScore(),false ) ) )
            .build();
    connection::iroha::PeerService::Sumeragi::send( getMyPublicKey(), txPeer );
}
void PeerServiceConfig::toIssue_distructPeer( const std::string &publicKey ) {
    if( !isExistPublicKey(publicKey) ) return;
    auto txPeer = TransactionBuilder<Update<Peer>>()
            .setSenderPublicKey(getMyPublicKey())
            .setPeer(txbuilder::createPeer(publicKey, peer::Node::defaultIP(), txbuilder::createTrust(-1.0, true)))
            .build();
    connection::iroha::PeerService::Sumeragi::send( getMyPublicKey(), txPeer );
}
void PeerServiceConfig::toIssue_removePeer( const std::string &publicKey ) {
    if( !isExistPublicKey(publicKey) ) return;
    auto txPeer = TransactionBuilder<Remove<Peer>>()
            .setSenderPublicKey(getMyPublicKey())
            .setPeer(txbuilder::createPeer(publicKey, peer::Node::defaultIP(), txbuilder::createTrust(-getMaxTrustScore(), false)))
            .build();
    connection::iroha::PeerService::Sumeragi::send( getMyPublicKey(), txPeer );
}
void PeerServiceConfig::toIssue_creditPeer( const std::string &publicKey ) {
    if( !isExistPublicKey(publicKey) ) return;
    if( findPeerPublicKey(publicKey)->getTrustScore() == getMaxTrustScore() ) return;
    auto txPeer = TransactionBuilder<Update<Peer>>()
            .setSenderPublicKey(getMyPublicKey())
            .setPeer(txbuilder::createPeer(publicKey, peer::Node::defaultIP(),
                                           txbuilder::createTrust( +1.0, true)))
            .build();
    connection::iroha::PeerService::Sumeragi::send( getMyPublicKey(), txPeer );
}



bool PeerServiceConfig::addPeer( const peer::Node &peer ) {
  try {
    if( isExistIP( peer.getIP() ) )
      throw exception::service::DuplicationIPException(peer.getIP());
    if( isExistPublicKey( peer.getPublicKey() ) )
      throw exception::service::DuplicationPublicKeyException(peer.getPublicKey());
    peerList.emplace_back( std::move(peer));
  } catch( exception::service::DuplicationPublicKeyException& e ) {
    logger::warning("addPeer") << e.what();
    return false;
  } catch( exception::service::DuplicationIPException& e ) {
    logger::warning("addPeer") << e.what();
    return false;
  }
  return true;
}

bool PeerServiceConfig::removePeer( const std::string& publicKey ) {
  try {
    auto it = findPeerPublicKey( publicKey );
    if ( !isExistPublicKey( publicKey ) )
      throw exception::service::UnExistFindPeerException(publicKey);
    peerList.erase(it);
  } catch (exception::service::UnExistFindPeerException& e) {
    logger::warning("removePeer") << e.what();
    return false;
  }
  return true;
}

bool PeerServiceConfig::updatePeer( const std::string& publicKey, const peer::Node& peer ) {
  try {
    auto it = findPeerPublicKey( publicKey );
    if (it == peerList.end() )
      throw exception::service::UnExistFindPeerException( publicKey );

    if ( !peer.isDefaultPublicKey() ) {
      auto upd_it = findPeerPublicKey(peer.getPublicKey());
      if( upd_it != it && upd_it != peerList.end() ) throw exception::service::DuplicationPublicKeyException(peer.getPublicKey());
      it->setPublicKey( peer.getPublicKey() );
    }

    if ( !peer.isDefaultIP() ) {
        auto upd_it = findPeerIP(peer.getIP());
        if( upd_it != it && upd_it != peerList.end() ) throw exception::service::DuplicationIPException(peer.getIP());
        it->setIP( peer.getIP() );
    }

    if ( it->getTrustScore() != 0.0 ) {
        it->setTrustScore( std::min( getMaxTrustScore(), it->getTrustScore()+peer.getTrustScore()) );
    }

    if( it->isOK() != peer.isOK() ) {
        it->setOK( peer.isOK() );
    }

  } catch ( exception::service::UnExistFindPeerException& e ) {
    logger::warning("updatePeer") << e.what();
    return false;
  } catch( exception::service::DuplicationPublicKeyException& e ) {
      logger::warning("updatePeer") << e.what();
    return false;
  } catch ( exception::service::DuplicationIPException& e ) {
    logger::warning("updatePeer") << e.what();
    return false;
  }
  return true;
}

//invoke next to addPeer
bool PeerServiceConfig::sendAllTransactionToNewPeer( const peer::Node& peer ) {
    logger::debug("peer-service") << "in sendAllTransactionToNewPeer";
    // when my node is not active, it don't send data.
    if( !(findPeerPublicKey( getMyPublicKey() )->isOK()) ) return false;

    uint64_t code = 0UL;
    {   // Send PeerList data ( Reason: Can't do to construct peerList for only transaction infomation. )
        logger::debug("peer-service") << "send all peer infomation";
        auto sorted_peerList = getPeerList();
        auto txResponse = Api::TransactionResponse();
        txResponse.set_message( "Initilize send now Active PeerList info" );
        txResponse.set_code( code++ );
        for (auto &&peer : sorted_peerList) {
            auto txPeer = TransactionBuilder<Add<Peer>>()
                    .setSenderPublicKey(getMyPublicKey())
                    .setPeer(txbuilder::createPeer(peer->getPublicKey(), peer->getIP(),
                                                   txbuilder::createTrust(getMaxTrustScore(), true)))
                    .build();
            txResponse.add_transaction()->CopyFrom(txPeer);
        }
        if( !connection::iroha::PeerService::Izanami::send( peer.getIP(), txResponse ) ) return false;
    }

    if(0){   // WIP(leveldb don't active) Send transaction data separated block to new peer.
        logger::debug("peer-service") << "send all transaction infomation";
        auto transactions = repository::transaction::findAll();
        int block_size = 500;
        for (int i = 0; i < transactions.size(); i += block_size) {
            auto txResponse = Api::TransactionResponse();
            txResponse.set_message("Midstream send Transactions");
            txResponse.set_code(code++);
            for (int j = i; j < i + block_size; j++) {
                txResponse.add_transaction()->CopyFrom(transactions[j]);
            }
            if (!connection::iroha::PeerService::Izanami::send(peer.getIP(), txResponse)) return false;
        }
    }

    {   // end-point
        logger::debug("peer-service") << "send end-point";
        auto txResponse = Api::TransactionResponse();
        txResponse.set_message("Finished send Transactions");
        txResponse.set_code(code++);
        if (!connection::iroha::PeerService::Izanami::send(peer.getIP(), txResponse)) return false;
    }
    return true;
}

bool PeerServiceConfig::validate_addPeer( const peer::Node& peer ) {
    try {
        if( isExistIP( peer.getIP() ) )
            throw exception::service::DuplicationIPException(std::move(peer.getIP()));
        if( isExistPublicKey( peer.getPublicKey() ) )
            throw exception::service::DuplicationPublicKeyException(std::move(peer.getPublicKey()));
    } catch( exception::service::DuplicationPublicKeyException& e ) {
        logger::warning("validate addPeer") << e.what();
        return false;
    } catch( exception::service::DuplicationIPException& e ) {
        logger::warning("validate addPeer") << e.what();
        return false;
    }
    return true;
}
bool PeerServiceConfig::validate_removePeer( const std::string &publicKey ) {
    try {
        if ( !isExistPublicKey( publicKey ) )
            throw exception::service::UnExistFindPeerException(publicKey);
    } catch (exception::service::UnExistFindPeerException& e) {
        logger::warning("validate removePeer") << e.what();
        return false;
    }
    return true;
}
bool PeerServiceConfig::validate_updatePeer( const std::string& publicKey, const peer::Node& peer ) {
    try {
        auto it = findPeerPublicKey( publicKey );
        if (it == peerList.end() )
            throw exception::service::UnExistFindPeerException( publicKey );

        if ( !peer.isDefaultPublicKey() ) {
            auto upd_it = findPeerPublicKey(peer.getPublicKey());
            if( upd_it != it && upd_it != peerList.end() ) throw exception::service::DuplicationPublicKeyException(peer.getPublicKey());
        }

        if ( !peer.isDefaultIP() ) {
            auto upd_it = findPeerIP(peer.getIP());
            if( upd_it != it && upd_it != peerList.end() ) throw exception::service::DuplicationIPException(peer.getIP());
        }

    } catch ( exception::service::UnExistFindPeerException& e ) {
        logger::warning("updatePeer") << e.what();
        return false;
    } catch( exception::service::DuplicationPublicKeyException& e ) {
        logger::warning("updatePeer") << e.what();
    } catch( exception::service::DuplicationIPException& e ) {
        logger::warning("updatePeer") << e.what();
        return false;
    }
    return true;
}


bool PeerServiceConfig::isLeaderMyPeer() {
  auto sorted_peers = getPeerList();
  if( sorted_peers.empty() ) return false;
  return (*sorted_peers.begin())->getPublicKey() == getMyPublicKey() &&
         (*sorted_peers.begin())->getIP() == getMyIp();
}

std::unique_ptr<peer::Node> PeerServiceConfig::leaderPeer() {
  return std::move( *getPeerList().begin() );
}
