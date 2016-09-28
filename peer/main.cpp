#include <thread>
#include <signal.h>
#include <unistd.h>
#include <atomic>

#include "../core/server/http_server.hpp"
#include "../core/consensus/connection/connection.hpp"
#include "../core/consensus/sumeragi.hpp"

#include "../core/util/yaml_loader.hpp"
#include "../core/util/logger.hpp"

#include "../core/service/peer_service.hpp"

std::atomic_bool running(true); 

void server(){
  http::server();
}

void sigIntHandler(int param){
  running = false;
  logger::info("main", "will halt");
}

int main() {

  signal(SIGINT, sigIntHandler);

  if(getenv("IROHA_HOME") == nullptr){
    logger::fital(__func__,"You must set $IROHA_HOME!");
    return 1;
  }

  std::cout<<"Process ID is "<< getpid() << std::endl;
  /*
  std::unique_ptr<connection::Config> config;
  std::unique_ptr<yaml::YamlLoader>   yamlLoader(new yaml::YamlLoader(std::string(getenv("IROHA_HOME"))+"/config/config.yml"));

  config->address = std::move(yamlLoader->get<std::string>("mediaDriver", "address"));
  config->port    = std::move(yamlLoader->get<std::string>("mediaDriver", "port"));
  config->name    = std::move(yamlLoader->get<std::string>("mediaDriver", "name"));
  config->publishStreamId    = yamlLoader->get<int>("mediaDriver", "publishStreamId");
  config->subscribeStreamId  = yamlLoader->get<int>("mediaDriver", "subscribeStreamId");
  
  connection::initialize_peer(std::move(config));
  */
 // std::cout << "(Second) Process ID is " << getpid() << std::endl;
 // std::unique_ptr<yaml::YamlLoader>   yaml(new yaml::YamlLoader(std::string(getenv("IROHA_HOME"))+"/config/config.yml"));
  std::string myPublicKey = peer::getMyPublicKey(); //yaml->get<std::string>("peer", "publicKey"); 
  std::vector<std::unique_ptr<peer::Node>> peer;
  peer.push_back(std::unique_ptr<peer::Node>(new peer::Node("1.2.5.6","AAA")));
  peer.push_back(std::unique_ptr<peer::Node>(new peer::Node("1.2.5.6","AAA")));
  peer.push_back(std::unique_ptr<peer::Node>(new peer::Node("1.2.5.6","AAA")));

  sumeragi::initializeSumeragi(myPublicKey, std::move(peer));

  std::thread http_th( server );
  std::thread sumeragi_th(sumeragi::loop);

  while(running){}

  http_th.detach();
  sumeragi_th.detach();
  
  return 0;
}

