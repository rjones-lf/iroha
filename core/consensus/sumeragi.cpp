/*
Copyright Soramitsu Co., Ltd. 2016 All Rights Reserved.
http://soramitsu.co.jp
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
         http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "sumeragi.hpp"
#include <queue>
#include <map>
#include <thread>
#include <cmath>

#include "../util/logger.hpp"
#include "../repository/consensus/merkle_transaction_repository.hpp"
#include "../repository/consensus/event_repository.hpp"
#include "../crypto/hash.hpp"
#include "../crypto/signature.hpp"

#include "../validation/transaction_validator.hpp"
#include "../service/peer_service.hpp"
#include "./connection/connection.hpp"
#include "../model/objects/asset.hpp"
#include "../model/objects/domain.hpp"
#include "../model/commands/transfer.hpp"
#include "../service/json_parse_with_json_nlohman.hpp"
#include "../service/json_parse.hpp"

/**
* |ーーー|　|ーーー|　|ーーー|　|ーーー|
* |　ス　|ー|　メ　|ー|　ラ　|ー|　ギ　|
* |ーーー|　|ーーー|　|ーーー|　|ーーー|
*
* A chain-based byzantine fault tolerant consensus algorithm, based in large part on BChain:
*
* Duan, S., Meling, H., Peisert, S., & Zhang, H. (2014). Bchain: Byzantine replication with
* high throughput and embedded reconfiguration. In International Conference on Principles of
* Distributed Systems (pp. 91-106). Springer.
*/
namespace sumeragi {

    using Object = json_parse::Object;
    using event::ConsensusEvent;
    using transaction::Transaction;
    using command::Transfer;
    using command::Add;

    struct Context {
        bool isSumeragi; // am I the leader or am I not?
        unsigned long maxFaulty;  // f
        unsigned long proxyTailNdx;
        int panicCount;
        unsigned long numValidatingPeers;
        std::string myPublicKey;

        std::vector<
                std::unique_ptr<peer::Node>
        > validatingPeers;

        Context(std::vector<
                std::unique_ptr<peer::Node>
        > peers):
                validatingPeers(std::move(peers))
        {}
    };

    std::unique_ptr<Context> context = nullptr;

    void initializeSumeragi(const std::string& myPublicKey,
                            std::vector<std::unique_ptr<peer::Node>> peers) {

        logger::info("sumeragi", "initialize");
        //merkle_transaction_repository::initLeaf();

        context = std::make_unique<Context>(std::move(peers));
        peers.clear();

        // *****************************
        // WIP ReWrite in production!!
        // *****************************
        logger::info("sumeragi", "Sumeragi ip is " +  context->validatingPeers.at(0)->getIP());
        logger::info("sumeragi", "My ip address is " + peer::getMyIp());
        if(peer::getMyIp() == context->validatingPeers.at(0)->getIP()){
            context->isSumeragi = true;
            logger::info("sumeragi", "===+==========+===");
            logger::info("sumeragi", "   |  |=+=|   |");
            logger::info("sumeragi", "  -+----------+-");
            logger::info("sumeragi", "   |          |");
            logger::info("sumeragi", "   |  I  am   |");
            logger::info("sumeragi", "   | Sumeragi |");
            logger::info("sumeragi", "   |          |");
            logger::info("sumeragi", "   A          A");
            
        }
        logger::info("sumeragi", "Sumeragi setted");

        logger::info("sumeragi", "set number of validatingPeer");
        context->numValidatingPeers = context->validatingPeers.size();
        context->maxFaulty = context->numValidatingPeers / 3;  // Default to approx. 1/3 of the network. TODO: make this configurable

        context->proxyTailNdx = context->maxFaulty*2 + 1;
        if (context->proxyTailNdx >= context->validatingPeers.size()) {
            context->proxyTailNdx = context->validatingPeers.size()-1;
        }

        context->panicCount = 0;
        context->myPublicKey = myPublicKey;

        connection::receive([&](std::string from, std::string message){
            logger::info("sumeragi", "received message! [" + message +"]");
            if(message.find("Transfer") != std::string::npos){
                auto ex = json_parse_with_json_nlohman::parser::load<
                    ConsensusEvent<
                        Transaction<
                            Transfer<object::Asset>
                        >
                    >
                >(message);
                logger::info("sumeragi", "hash is "+ ex->getHash());
                // WIP currently, unuse hash in event repository,
                repository::event::add( "dummy", std::move(ex));
            }
        });

        logger::info("sumeragi", "initialize numValidatingPeers :" + std::to_string(context->numValidatingPeers));
        logger::info("sumeragi", "initialize maxFaulty :" + std::to_string(context->maxFaulty));
        logger::info("sumeragi", "initialize proxyTailNdx :" + std::to_string(context->proxyTailNdx));

        logger::info("sumeragi", "initialize panicCount :" + std::to_string(context->panicCount));
        logger::info("sumeragi", "initialize myPublicKey :" + context->myPublicKey);

        //TODO: move the peer service and ordering code to another place
        determineConsensusOrder(); // side effect is to modify validatingPeers
        logger::info("sumeragi", "initialize is sumeragi :" + std::to_string(context->isSumeragi));
        logger::info("sumeragi", "initialize.....  complete!");
    }

    unsigned long long getNextOrder() {
        return 0l;
        //return merkle_transaction_repository::getLastLeafOrder() + 1;
    }

    void processTransaction(const std::unique_ptr<event::Event>& event) {

        logger::info("sumeragi", "processTransaction()");
        //if (!transaction_validator::isValid(event->getTx())) {
        //    return; //TODO-futurework: give bad trust rating to nodes that sent an invalid event
        //}
        logger::info("sumeragi", "valied");

        event->addSignature(
            peer::getMyPublicKey(),
            signature::sign(
                    event->getHash(),
                    peer::getMyPublicKey(),
                    peer::getPrivateKey()
            )
        );

        logger::info("sumeragi", "context->isSumeragi :" + std::to_string(context->isSumeragi));

        if (event->eventSignatureIsEmpty() && context->isSumeragi) {
            logger::info("sumeragi", "signatures.empty() isSumragi");
            // Determine the order for processing this event
            event->order = getNextOrder();

            logger::info("sumeragi", "new  order:" + std::to_string(event->order));
        } else if (!event->eventSignatureIsEmpty()) {
            logger::info("sumeragi", "signatures.exist()");
            // Check if we have at least 2f+1 signatures needed for Byzantine fault tolerance
            if (event->getNumValidSignatures() >= context->maxFaulty*2 + 1) {
                logger::info("sumeragi", "event->getNumValidSignatures() >= context->maxFaulty*2 + 1");
                // Check Merkle roots to see if match for new state
                //TODO: std::vector<std::string>>const merkleSignatures = event.merkleRootSignatures;
                //Try applying transaction locally and compute the merkle root
                //std::unique_ptr<merkle_transaction_repository::MerkleNode> newRoot = merkle_transaction_repository::calculateNewRoot(event);
                //logger::info("sumeragi", "newRoot hash:"+newRoot->hash);
                //logger::info("sumeragi", "event hash:"+event->merkleRootHash);

                // See if the merkle root matches or not
                // if (newRoot->hash != event->merkleRootHash) {
                //    panic(event);
                //    return;
                // }

                // Commit locally
                logger::info("sumeragi", "commit");
                merkle_transaction_repository::commit(event); //TODO: add error handling in case not saved
            } else {
                // This is a new event, so we should verify, sign, and broadcast it
                event->addSignature( peer::getMyPublicKey(), signature::sign(event->getHash(), peer::getMyPublicKey(), peer::getPrivateKey()));
                if (context->validatingPeers.at(context->proxyTailNdx)->getPublicKey() == peer::getMyPublicKey()) {
                    connection::send(context->validatingPeers.at(context->proxyTailNdx)->getIP(), event->getHash()); // Think In Process
                } else {
                    connection::sendAll(event->getHash()); // TODO: Think In Process
                }

                setAwkTimer(3000, [&](){
                    if (!merkle_transaction_repository::leafExists(event->getHash())) {
                        panic(event);
                    }
                });
            }
        }
    }

    /**
    *
    * For example, given:
    * if f := 1, then
    *  _________________    _________________
    * /        A        \  /        B        \
    * |---|  |---|  |---|  |---|  |---|  |---|
    * | 0 |--| 1 |--| 2 |--| 3 |--| 4 |--| 5 |
    * |---|  |---|  |---|  |---|  |---|  |---|,
    *
    * if 2f+1 signature are not received within the timer's limit, then
    * the set of considered validators, A, is expanded by f (e.g., by 1 in the example below):
    *  ________________________    __________
    * /           A            \  /    B     \
    * |---|  |---|  |---|  |---|  |---|  |---|
    * | 0 |--| 1 |--| 2 |--| 3 |--| 4 |--| 5 |
    * |---|  |---|  |---|  |---|  |---|  |---|.
    */
    void panic(const std::unique_ptr<event::Event>& event) {
        context->panicCount++; // TODO: reset this later
        unsigned long broadcastStart = 2 * context->maxFaulty + 1 + context->maxFaulty * context->panicCount;
        unsigned long broadcastEnd = broadcastStart + context->maxFaulty;

        // Do some bounds checking
        if (broadcastStart > context->numValidatingPeers - 1) {
            broadcastStart = context->numValidatingPeers - 1;
        }

        if (broadcastEnd > context->numValidatingPeers - 1) {
            broadcastEnd = context->numValidatingPeers - 1;
        }
        logger::info( "sumeragi", "broadcastEnd:"+ std::to_string(broadcastEnd));
        logger::info( "sumeragi", "broadcastStart:"+ std::to_string(broadcastStart));
        connection::sendAll(event->getHash()); //TODO: change this to only broadcast to peer range between broadcastStart and broadcastEnd
    }

    void setAwkTimer(int const sleepMillisecs, std::function<void(void)> const action) {
        std::thread([action, sleepMillisecs]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(sleepMillisecs));
            action();
        }).join();
    }

    /**
     * The consensus order is based primarily on the trust scores. If two trust scores
     * are the same, then the order (ascending) of the public keys for the servers are used.
     */
    void determineConsensusOrder() {
        std::sort(context->validatingPeers.begin(), context->validatingPeers.end(),
              [](const std::unique_ptr<peer::Node> &lhs,
                 const std::unique_ptr<peer::Node> &rhs) {
                  return lhs->getTrustScore() > rhs->getTrustScore()
                         || (lhs->getTrustScore() == rhs->getTrustScore()
                             && lhs->getPublicKey() < rhs->getPublicKey());
              }
        );

        logger::info("sumeragi", "determineConsensusOrder sorted!");
        logger::info("sumeragi", "determineConsensusOrder myPubkey:"+context->myPublicKey);
        for(const auto& peer : context->validatingPeers) {
            logger::info("sumeragi", "determineConsensusOrder PublicKey:"+peer->getPublicKey());
            logger::info("sumeragi", "determineConsensusOrder ip:"+peer->getIP());
        }
        context->isSumeragi = context->validatingPeers.at(0)->getPublicKey() == context->myPublicKey;
    }

    void loop() {
        logger::info("sumeragi", "=##################=");
        logger::info("sumeragi", "start main loop");

        while (true) {  // TODO: replace with callback linking the event repository?
            if(!repository::event::empty()) {
                // Determine node order
                determineConsensusOrder();

                logger::info("sumeragi", "event queue not empty");

                auto&& events = repository::event::findAll();
                logger::info("sumeragi", "event's size " + std::to_string(events.size()));
                
                // Sort the events to determine priority to process
                std::sort(events.begin(), events.end(),
                          [&](const auto &lhs,
                             const auto &rhs) {
                              logger::info("sumeragi", "lhs->getNumValidSignatures:" + std::to_string(lhs->getNumValidSignatures()));
                              logger::info("sumeragi", "rhs->getNumValidSignatures:" + std::to_string(rhs->getNumValidSignatures()));
                              return lhs->getNumValidSignatures() > rhs->getNumValidSignatures()
                                     || (context->isSumeragi && lhs->order == 0)
                                     || lhs->order < rhs->order;
                          }
                );

                logger::info("sumeragi", "sorted " + std::to_string(events.size()));
                for (auto&& event : events) {

                    logger::info("sumeragi", "evens order:" + std::to_string(event->order));
                    //if (!transaction_validator::isValid(event->getTx())) {
                    //    continue;
                    //}

                    // Process transaction
                    processTransaction(std::move(event));
                }
            }
        }
    }

};  // namespace sumeragi
