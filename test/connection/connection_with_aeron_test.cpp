#include "../../core/consensus/connection/connection.hpp"

#include <string>
#include <iostream>

#include <unordered_map>

int main(int argc, char* argv[]){
    if(argc != 3){
        return 1;
    }

    connection::initialize_peer(nullptr);

    if(std::string(argv[1]) == "sender"){
        connection::addPublication(argv[2]);
        while(1){
            connection::sendAll("Mizuki < nya-n!");
        }
    }else{
        connection::exec_subscription(argv[2]);
        connection::receive([](std::string from, std::string message){
            std::cout <<" receive :" << message <<" from:"<< from << "\n";
        });
    }
    while(1){}
    connection::finish();
    return 0;
}
