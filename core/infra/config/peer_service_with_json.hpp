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

#ifndef PEER_SERVICE_WITH_JSON_HPP
#define PEER_SERVICE_WITH_JSON_HPP

#include <vector>
#include <set>
#include <map>
#include <queue>
#include <service/peer_service.hpp>
#include "abstract_config_manager.hpp"

namespace config {

class PeerServiceConfig : config::AbstractConfigManager {
 private:
  PeerServiceConfig();

  std::string getMyPublicKeyWithDefault(const std::string& defaultValue);
  std::string getMyPrivateKeyWithDefault(const std::string& defaultValue);
  std::string getMyIpWithDefault(const std::string& defaultValue);
  double getMaxTrustScoreWithDefault(double defaultValue);
  size_t getMaxFaultyScoreWithDefault(size_t defaultValue);
  std::vector<json> getGroup();

 protected:
  void parseConfigDataFromString(std::string&& jsonStr) override;

 public:
  static PeerServiceConfig &getInstance();

 public:

 /*
   TODO: For ease of moving peer service to another class or namespace,
       peer service config is tempolary separeted from below.
  */

 private:
  static std::vector<peer::Node> peerList;
  bool is_active;

  void initialziePeerList_from_json();

  std::vector<peer::Node>::iterator findPeerIP( const std::string& ip );
  std::vector<peer::Node>::iterator findPeerPublicKey( const std::string& publicKey );

 public:
  std::string getMyPublicKey();
  std::string getMyPrivateKey();
  std::string getMyIp();
  double getMaxTrustScore();
  size_t getMaxFaulty();

  bool isMyActive();
  void activate();
  void stop();


  std::vector<std::unique_ptr<peer::Node>> getPeerList();
  std::vector<std::string> getIpList();

  // is exist which peer?
  bool isExistIP( const std::string& );
  bool isExistPublicKey( const std::string& );

  // check are broken? peer
  void checkBrokenPeer( const std::string& ip );

  // Initialize
  void finishedInitializePeer();

  // invoke to issue transaction
  void toIssue_addPeer( const peer::Node& );
  void toIssue_distructPeer( const std::string &publicKey );
  void toIssue_removePeer( const std::string &publicKey );
  void toIssue_creditPeer( const std::string &publicKey );

  // invoke when execute transaction
  bool addPeer( const peer::Node& );
  bool removePeer( const std::string &publicKey );
  bool updatePeer( const std::string& publicKey, const peer::Node& peer );

  //invoke next to addPeer
  bool sendAllTransactionToNewPeer( const peer::Node& );

  // invoke when validator transaction
  bool validate_addPeer( const peer::Node& );
  bool validate_removePeer( const std::string &publicKey );
  bool validate_updatePeer( const std::string& publicKey, const peer::Node& peer );

  // equatl to isSumeragi
  bool isLeaderMyPeer();
  std::unique_ptr<peer::Node> leaderPeer();

  virtual std::string getConfigName();
};
}

#endif  // PEER_SERVICE_WITH_JSON_HPP
