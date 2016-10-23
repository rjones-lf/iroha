#include <vector>
#include <string>
#include <memory>

#include "../util/yaml_loader.hpp"
#include "peer_service.hpp"

namespace peer{

    std::string Node::getIP() const{
        return ip;
    }

    std::string Node::getPublicKey() const{
        return publicKey;
    }

    double Node::getTrustScore() const {
        return 0; // TODO: WIP
    }

    std::string getMyPublicKey() {
        return "Base64";// TODO: WIP
    }

    std::string getPrivateKey() {
        return "Base64";// TODO: WIP
    }

    std::vector<Node> getPeerList() {  
        std::vector<Node> res;
        std::unique_ptr<yaml::YamlLoader> yaml(new yaml::YamlLoader(std::string(getenv("IROHA_HOME")) + "/config/config.yml"));
        auto nodes = yaml->get<std::vector<peer::Node> >("peer", "node");
        for (std::size_t i=0;i < nodes.size();i++) {
            res.push_back( nodes[i] );
        }
        return res;
    }
};
