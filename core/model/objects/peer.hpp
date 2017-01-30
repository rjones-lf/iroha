/*
Copyright Soramitsu Co., Ltd. 2016 All Rights Reserved.
http://soramitsu.co.jp

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

#ifndef CORE_DOMAIN_OBJECTS_PEER_HPP_
#define CORE_DOMAIN_OBJECTS_PEER_HPP_

#include <string>
#include <memory>

namespace object {

class Peer {
private:
    std::string     ip;
    std::string     publicKey;
    double          trustScore;

public:
    explicit Peer():
        ip(""),
        publicKey(""),
        trustScore(1.0)
    {}

    explicit Peer(
        std::string ip,
        std::string publicKey
    );

    explicit Peer(
        std::string     ip,
        std::string     publicKey,
        double          trustScore
    );
    
    
    ~Peer() = default; // make dtor virtual
    Peer(Peer&&) = default;  // support moving
    Peer& operator = (Peer&&) = default;
    Peer(const Peer&) = default; // support copying
    Peer& operator = (const Peer&) = default;

    bool operator < (const Peer& peer) const {
        if( ip == peer.ip ) return publicKey < peer.publicKey;
        return ip < peer.ip;
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

    Peer getInstance() const;

};

};  // namespace peer

#endif  // CORE_DOMAIN_OBJECTS_PEER_HPP_

