#include <vector>
#include <string>
#include <memory>

#include "peer_service.hpp"

namespace peer{

    std::string Node::getIP() const{
        return ip;
    }

    std::string Node::getPublicKey() const{
        return publicKey;
    }

    std::string getMyPublicKey() {
        return "Base64";// WIP
    }

    std::string getPrivateKey() {
        return "Base64";// WIP
    }

    std::vector<Node> getPeerList() {  
        std::vector<Node> res;
        // WIP use json loadr
        return res;
    }
};
