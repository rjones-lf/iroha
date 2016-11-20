
#include "../../service/peer_service.hpp"

#include <iostream>  // for debug writing

#include <fstream>   // ifstream, ofstream
#include <sstream>   // istringstream
#include <json.hpp>
#include <vector>

namespace peer {

    using json = nlohmann::json;

    std::string openConfig(){
        if(std::string(getenv("IROHA_HOME")) == ""){
            std::cerr << "IROHA_HOMEをセットして" << std::endl;
        }
        std::ifstream ifs(std::string(getenv("IROHA_HOME"))+"/config/sumeragi.json");
        if(ifs.fail()) {
            std::cerr << "Fileが見つかりません" << std::endl;
            return "";
        }
        std::istreambuf_iterator<char> it(ifs);
        std::istreambuf_iterator<char> last;
        std::string res(it, last);
        return res;
    }

    std::string getMyPublicKey(){
        try{
            auto data = json::parse(openConfig());
            std::cout << data.dump() <<std::endl;
            return data["me"]["publicKey"].get<std::string>();
        }catch(...){
            return "";
        }
    }

    std::string getPrivateKey(){
        try{
            auto data = json::parse(openConfig());    
            return data["me"]["privateKey"].get<std::string>();
        }catch(...){
            return "";
        }
    }

    std::string getMyIp() {
        try{
            auto data = json::parse(openConfig());
            return data["me"]["ip"].get<std::string>();
        }catch(...){
            return "";
        }
    }

    std::vector<std::unique_ptr<peer::Node>> getPeerList(){
        std::vector<std::unique_ptr<peer::Node>> nodes;
        try{
            auto data = json::parse(openConfig());
            for(const auto& peer : data["group"].get<std::vector<json>>()){
                nodes.push_back(std::make_unique<peer::Node>(
                    peer["ip"].get<std::string>(),
                    peer["publicKey"].get<std::string>(),
                    1
                ));
            }
            return std::move(nodes);
        }catch(...){
            return std::move(nodes);
        }
    }
};