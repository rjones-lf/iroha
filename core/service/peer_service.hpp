#ifndef __CORE_URL_SERVICE_HPP__
#define __CORE_URL_SERVICE_HPP__

#include <vector>
#include <string>

#include "../util/yaml_loader.hpp"


namespace peer{

    class Node {
        public:
            std::string ip;
            std::string publicKey;
            double trustScore;

            Node(){}

            Node(
                std::string myIP,
                std::string myPubKey,
                double myTrustScore
            ):
                ip(myIP),
                publicKey(myPubKey),
                trustScore(myTrustScore)
            {}
            
            
            ~Node() = default; // make dtor virtual
            Node(Node&&) = default;  // support moving
            Node& operator = (Node&&) = default;
            Node(const Node&) = default; // support copying
            Node& operator = (const Node&) = default;
           

            std::string getIP() const;
            std::string getPublicKey() const;
            double getTrustScore() const;
    };


    std::string getMyPublicKey();

    std::string getPrivateKey();

    std::vector<Node> getPeerList();
};

#endif
