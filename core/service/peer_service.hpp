#ifndef __CORE_URL_SERVICE_HPP__
#define __CORE_URL_SERVICE_HPP__

#include <vector>
#include <string>

namespace peer{

    class Node {
        public:
            std::string ip;
            std::string publicKey;

            Node(){}

            Node(
                std::string aip,
                std::string apubkey
            ):
                ip(aip),
                publicKey(apubkey)
            {}
            
            
            ~Node() = default; // make dtor virtual
            Node(Node&&) = default;  // support moving
            Node& operator = (Node&&) = default;
            Node(const Node&) = default; // support copying
            Node& operator = (const Node&) = default;
           

            std::string getIP() const;
            std::string getPublicKey() const;
    };


    std::string getMyPublicKey();

    std::string getPrivateKey();

    std::vector<Node> getPeerList();
};

#endif
