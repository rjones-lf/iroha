
#include "../../service/peer_service.hpp"
#include "../../util/logger.hpp"

#include <iostream>  // for debug writing

#include <fstream>   // ifstream, ofstream
#include <sstream>   // istringstream
#include <json.hpp>
#include <vector>

namespace peer {

    using json = nlohmann::json;
    static json configData;

    json openConfig(){
        if(configData.empty()) {
            try{
                if (std::string(getenv("IROHA_HOME")) == "") {
                    std::cerr << "IROHA_HOMEをセットして" << std::endl;
                }
                std::ifstream ifs(std::string(getenv("IROHA_HOME")) + "/config/sumeragi.json");
                if (ifs.fail()) {
                    std::cerr << "Fileが見つかりません" << std::endl;
                    return json();
                }
                std::istreambuf_iterator<char> it(ifs);
                std::istreambuf_iterator<char> last;
                std::string res(it, last);
                logger::info("peer with json", "load json is "+ res);
                configData = json::parse(res);
                return configData;
            }catch(...){
                logger::error("peer with json", "Bad json!!");
                return json();
            }
        }else{
            return configData;
        }
    }

    std::string getMyPublicKey(){
        try{
            return openConfig()["me"]["publicKey"].get<std::string>();
        }catch(...){
            return "";
        }
    }

    std::string getPrivateKey(){
        try{
            return openConfig()["me"]["privateKey"].get<std::string>();
        }catch(...){
            return "";
        }
    }

    std::string getMyIp() {
        try{
            return openConfig()["me"]["ip"].get<std::string>();
        }catch(...){
            return "";
        }
    }

    std::vector<std::unique_ptr<peer::Node>> getPeerList(){
        std::vector<std::unique_ptr<peer::Node>> nodes;
        try{
            for(const auto& peer : openConfig()["group"].get<std::vector<json>>()){
                nodes.push_back(std::make_unique<peer::Node>(
                    peer["ip"].get<std::string>(),
                    peer["publicKey"].get<std::string>(),
                    1
                ));
            }
            return nodes;
        }catch(...){
            return nodes;
        }
    }
};
