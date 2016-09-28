#include <vector>
#include <string>
#include <memory>
#include <fstream>
#include <iostream>
#include <iterator>

#include "../util/yaml_loader.hpp"
#include "peer_service.hpp"

namespace peer{

    std::string Node::getIP() const{
        return ip;
    }

    std::string Node::getPublicKey() const{
        return publicKey;
    }

    // Ah^~, I want to separate file loader... 
    std::string getMyPublicKey() {
        std::ifstream ifs(std::string(getenv("IROHA_HOME")) + "/config/public.key");
        if (ifs.fail()){
            // WIP Please could you insert generate key pair
            return "";
        }
        std::string str(
            (std::istreambuf_iterator<char>(ifs)),
            std::istreambuf_iterator<char>()
        );
    }
    std::string getPrivateKey() {
        std::ifstream ifs(std::string(getenv("IROHA_HOME")) + "/config/private.key");
        if (ifs.fail()){
            // WIP Please could you insert generate key pair
            return "";
        }
        std::string str(
            (std::istreambuf_iterator<char>(ifs)),
            std::istreambuf_iterator<char>()
        );
        return str;
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
