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

#ifndef __CORE_PEER_SERVICE_HPP__
#define __CORE_PEER_SERVICE_HPP__

#include <memory>
#include <string>
#include <vector>

namespace peer {

inline static const std::string defaultLedgerName() { return ""; }

inline static const std::string defaultIP() { return ""; }

inline static const std::string defaultPubKey() { return ""; }

struct Node {
  std::string ledger_name;
  std::string publicKey;
  std::string ip;
  double trust;
  bool active;
  bool join_ledger;

  Node(std::string myIP = defaultIP(),
       std::string myPubKey = defaultPubKey(),
       double myTrustScore = 100.0,
       std::string ledger_name = defaultLedgerName(),
       bool active = false, bool join_ledger = false)
      : ledger_name(ledger_name),
        publicKey(myPubKey),
        ip(myIP),
        trust(myTrustScore),
        active(active),
        join_ledger(join_ledger) {}

  bool isDefaultIP() const { return ip == defaultIP(); }
  bool isDefaultPubKey() const { return publicKey == defaultPubKey(); }
};

using Nodes = std::vector<std::shared_ptr<Node>>;

namespace myself {

std::string getPublicKey();
std::string getPrivateKey();
std::string getIp();

bool isActive();
void activate();
void stop();

// equatl to isSumeragi
bool isLeader();

}  // namespace myself

namespace service {

void initialize();

size_t getMaxFaulty();
Nodes getAllPeerList();
Nodes getActivePeerList();
std::vector<std::string> getIpList();

// is exist which peer?
bool isExistIP(const std::string &);
bool isExistPublicKey(const std::string &);

Nodes::iterator findPeerIP(const std::string &ip);
Nodes::iterator findPeerPublicKey(const std::string &publicKey);
std::shared_ptr<peer::Node> leader();

}  // namespace service

namespace transaction {
namespace isssue {

// invoke to issue transaction
void add(const peer::Node &);                            // TODO
void remove(const std::string &);                        // TODO
void setTrust(const std::string &, const double &);      // TODO
void changeTrust(const std::string &, const double &);   // TODO
void setActive(const std::string &, const bool active);  // TODO

}  // namespace isssue

namespace executor {
// invoke when execute transaction
bool add(const peer::Node &);
bool remove(const std::string &);
bool setTrust(const std::string &, const double &);
bool changeTrust(const std::string &, const double &);
bool setActive(const std::string &, const bool active);
}  // namespace executor

namespace validator {
// invoke when validator transaction
bool add(const peer::Node &);
bool remove(const std::string &);
bool setTrust(const std::string &, const double &);
bool changeTrust(const std::string &, const double &);
bool setActive(const std::string &, const bool active);
}  // namespace validator
}  // namespace transaction
}  // namespace peer

#endif
