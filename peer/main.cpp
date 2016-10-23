#include <thread>
#include <signal.h>
#include <unistd.h>
#include <atomic>

#include <json.hpp>

#include "../core/server/http_server.hpp"
#include "../core/consensus/connection/connection.hpp"
#include "../core/consensus/sumeragi.hpp"
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
    std::cout << "You must set IROHA_HOME!" << std::endl;
    return 1;
  }

  std::cout<<"Process ID is "<< getpid() << std::endl;

  std::unique_ptr<connection::Config> config;

  config->ip_addr = "WIP";
  config->port    = "WIP";
  config->name    = "WIP";

  std::string myPublicKey = "WIP";


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

