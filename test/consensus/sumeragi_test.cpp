#include "../../core/consensus/sumeragi.hpp"
#include "../../core/repository/consensus/event_repository.hpp"
#include "../../core/model/transactions/transfer_transaction.hpp"

#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <thread>

/*
 Sample Public key
 UKsNazVWDZ1oqNBoFpckirYDvwI/LyL9BqNYXVWCqmw=
 */

/*
 Sample Friend Key
 public: fccpkrZyLlxJUQm8RpJXedWVZfbg2Dde0iPphwD+jQ0=
*/
int main(){
    std::string cmd;
    std::vector<std::unique_ptr<peer::Node>> nodes;
    nodes.push_back(std::make_unique<peer::Node>(
        "45.32.158.71",
        "fccpkrZyLlxJUQm8RpJXedWVZfbg2Dde0iPphwD+jQ0=",
        1.0
    ));
    std::string pubKey = "UKsNazVWDZ1oqNBoFpckirYDvwI/LyL9BqNYXVWCqmw=";
    sumeragi::initializeSumeragi( pubKey, std::move(nodes));
    std::thread http_th( []() {
        sumeragi::loop();
    });

    while(1){
        std::cout << "in >> ";
        std::cin>> cmd;
        if(cmd == "quit") break;
        repository::event::add("hash", std::make_unique<consensus_event::ConsensusEvent>(
            std::make_unique<transaction::TransferTransaction>(
                "fccpkrZyLlxJUQm8RpJXedWVZfbg2Dde0iPphwD+jQ0=",
                pubKey,
                "test",
                cmd
            )
        ));

    }

    http_th.detach();

    return 0;
}
