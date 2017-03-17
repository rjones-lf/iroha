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

        static std::string defaultIP() {
            return "";
        }
        static std::string defaultPublicKey() {
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
            return ip == "";
        }
        bool isDefaultPublicKey() const {
            return publicKey == "";
        }

    };
}

#endif
