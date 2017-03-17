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

#include <vector>
#include <string>
#include <memory>

namespace peer
{
    class Node {
        std::string ip;
        std::string publicKey;
        double trustScore;
        bool isok;

    public:

        inline static const std::string defaultIP() {
            return "";
        }
        inline static const std::string defaultPublicKey() {
            return "";
        }

        Node(
            std::string myIP = defaultIP(),
            std::string myPubKey = defaultPublicKey(),
            double myTrustScore = 1.0,
            bool isok = true
        ):
            ip(myIP),
            publicKey(myPubKey),
            trustScore(myTrustScore),
            isok( true )
        {}
        
        
        ~Node() = default; // make dtor virtual
        Node(Node&&) = default;  // support moving
        Node& operator = (Node&&) = default;
        Node(const Node&) = default; // support copying
        Node& operator = (const Node&) = default;


        bool operator < (const Node& node) const {
            return publicKey < node.getPublicKey();
        }
        bool operator == (const Node& node) const {
            return publicKey == node.getPublicKey();
        }

        std::string getIP() const {
            return ip;
        }

        std::string getPublicKey() const {
            return publicKey;
        }

        double getTrustScore() const {
            return trustScore;
        }

        bool isOK() const {
            return isok;
        }

        void setIP( const std::string& ip ) {
            this->ip = ip;
        }
        void setPublicKey( const std::string& publickey ) {
            this->publicKey = publickey;
        }
        void setTrustScore( const double& trustScore ) {
            this->trustScore = trustScore;
        }
        void setOK( const bool ok ) {
            this->isok = ok;
        }

        bool isDefaultIP() const {
            return ip == defaultIP();
        }
        bool isDefaultPublicKey() const {
            return publicKey == defaultPublicKey();
        }

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
    }

    namespace service {
        void initialize();//void initialziePeerList_from_json();
        size_t getMaxFaulty();
        Nodes getPeerList();
        std::vector<std::string> getIpList();
        // is exist which peer?
        bool isExistIP(const std::string &);
        bool isExistPublicKey(const std::string &);
        Nodes::iterator findPeerIP(const std::string &ip);
        Nodes::iterator findPeerPublicKey(const std::string &publicKey);
        std::shared_ptr<peer::Node> leaderPeer();
    }

    namespace transaction {

        // Initialize
        namespace izanami {
            void finished(); //void finishedInitializePeer();
            //invoke next to addPeer
            bool started(const Node& peer);//bool sendAllTransaction( const peer::Node& );
        }


        namespace isssue {
            // invoke to issue transaction
            void add(const peer::Node &);//void toIssue_addPeer( const peer::Node& );
            void distruct(const std::string &);//void toIssue_distructPeer( const std::string &publicKey );
            void remove(const std::string &);//void toIssue_removePeer( const std::string &publicKey );
            void credit(const std::string &);//void toIssue_creditPeer( const std::string &publicKey );
        }
        namespace executor {
            // invoke when execute transaction
            bool add(const peer::Node &);//bool addPeer( const peer::Node& );
            bool remove(const std::string &);//bool removePeer( const std::string &publicKey );
            bool update(const std::string &,
                        const peer::Node &);//bool updatePeer( const std::string& publicKey, const peer::Node& peer );
        }
        namespace validator {
            // invoke when validator transaction
            bool add(const peer::Node &);//bool validate_addPeer( const peer::Node& );
            bool remove(const std::string &);//bool validate_removePeer( const std::string &publicKey );
            bool update(const std::string &,
                        const peer::Node &);//bool validate_updatePeer( const std::string& publicKey, const peer::Node& peer );
        }
    }


}

#endif
