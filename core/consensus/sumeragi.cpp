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
#include <atomic>
#include <deque>
#include <cmath>

#include <thread_pool.hpp>

#include "../util/logger.hpp"
#include "../repository/consensus/merkle_transaction_repository.hpp"
#include "../crypto/hash.hpp"
#include "../crypto/signature.hpp"

#include "../infra/protobuf/convertor.hpp"

#include "../validation/transaction_validator.hpp"
#include "../service/peer_service.hpp"
#include "./connection/connection.hpp"
#include "../model/objects/asset.hpp"
#include "../model/objects/domain.hpp"
#include "../model/commands/transfer.hpp"

#include "../repository/consensus/transaction_repository.hpp"
#include "../repository/domain/account_repository.hpp"
#include "../infra/config/peer_service_with_json.hpp"
#include "../infra/config/iroha_config_with_json.hpp"

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

    using event::ConsensusEvent;
    using transaction::Transaction;
    using namespace command;
    using namespace object;


    static size_t concurrency =
        config::IrohaConfigManager::getInstance().getParam("concurrency", 0) <= 0
        ? 1
        : std::thread::hardware_concurrency();



    //thread pool and a storage of events 
    static ThreadPool pool(
        ThreadPoolOptions{
            .threads_count = concurrency,
            .worker_queue_size = 1024
        }
    );


    namespace detail {

        std::uint32_t getNumValidSignatures(const Event::ConsensusEvent& event) {
            std::uint32_t sum = 0;
            for (auto&& esig: event.eventsignatures()) {
                if (signature::verify(esig.signature(), event.transaction().hash(), esig.publickey())) {
                    sum++;
                }
            }
            return sum;
        }

        void addSignature(Event::ConsensusEvent& event, const std::string& publicKey, const std::string& signature) {
            Event::EventSignature sig;
            sig.set_signature(signature);
            sig.set_publickey(publicKey);
            event.add_eventsignatures()->CopyFrom(sig);
        }

        bool eventSignatureIsEmpty(const Event::ConsensusEvent& event) {
            return event.eventsignatures_size() == 0;
        }

        void printIsSumeragi(bool isSumeragi) {
            if (isSumeragi){
                logger::explore("sumeragi") <<  "===+==========+===";
                logger::explore("sumeragi") <<  "   |  |=+=|   |";
                logger::explore("sumeragi") <<  "  -+----------+-";
                logger::explore("sumeragi") <<  "   |          |";
                logger::explore("sumeragi") <<  "   |  I  am   |";
                logger::explore("sumeragi") <<  "   | Sumeragi |";
                logger::explore("sumeragi") <<  "   |          |";
                logger::explore("sumeragi") <<  "   A          A";
            } else {
                logger::explore("sumeragi") <<  "   /\\         /\\";
                logger::explore("sumeragi") <<  "   ||  I  am  ||";
                logger::explore("sumeragi") <<  "   ||   peer  ||";
                logger::explore("sumeragi") <<  "   ||         ||";
                logger::explore("sumeragi") <<  "   AA         AA";
            }
        }

        void printJudge(int numValidSignatures, int numValidationPeer, int faulty) {
            for (int i=0; i<numValidationPeer; i++) {
                if (i < numValidSignatures){
                    logger::explore("sumeragi") <<  "\033[1m\033[92m+-ー-+\033[0m";
                    logger::explore("sumeragi") <<  "\033[1m\033[92m| 　 |\033[0m";
                    logger::explore("sumeragi") <<  "\033[1m\033[92m|-承-|\033[0m";
                    logger::explore("sumeragi") <<  "\033[1m\033[92m| 　 |\033[0m";
                    logger::explore("sumeragi") <<  "\033[1m\033[92m+-＝-+\033[0m";
                } else {
                    logger::explore("sumeragi") <<  "\033[91m+-ー-+\033[0m";
                    logger::explore("sumeragi") <<  "\033[91m| 　 |\033[0m";
                    logger::explore("sumeragi") <<  "\033[91m| 否 |\033[0m";
                    logger::explore("sumeragi") <<  "\033[91m| 　 |\033[0m";
                    logger::explore("sumeragi") <<  "\033[91m+-＝-+\033[0m";
                }
            }

            std::string line;
            for (int i=0; i<numValidationPeer; i++) line += "==＝==";
            logger::explore("sumeragi") <<  line;
            
            logger::explore("sumeragi") <<  "numValidSignatures:"
                                    <<  numValidSignatures
                                    <<  " faulty:"
                                    <<  faulty;
        }

        void printAgree() {
            logger::explore("sumeragi") <<  "\033[1m\033[92m+==ーー==+\033[0m";
            logger::explore("sumeragi") <<  "\033[1m\033[92m|+-ーー-+|\033[0m";
            logger::explore("sumeragi") <<  "\033[1m\033[92m|| 承認 ||\033[0m";
            logger::explore("sumeragi") <<  "\033[1m\033[92m|+-ーー-+|\033[0m";
            logger::explore("sumeragi") <<  "\033[1m\033[92m+==ーー==+\033[0m";
        }

        void printReject() {
            logger::explore("sumeragi") <<  "\033[91m+==ーー==+\033[0m";
            logger::explore("sumeragi") <<  "\033[91m|+-ーー-+|\033[0m";
            logger::explore("sumeragi") <<  "\033[91m|| 否認 ||\033[0m";
            logger::explore("sumeragi") <<  "\033[91m|+-ーー-+|\033[0m";
            logger::explore("sumeragi") <<  "\033[91m+==ーー==+\033[0m";
        }
    } // namespace detail

    struct Context {
        bool            isSumeragi; // am I the leader or am I not?
        std::uint64_t   maxFaulty;  // f
        std::uint64_t   proxyTailNdx;
        std::int32_t    panicCount;
        std::int64_t    commitedCount = 0;
        std::uint64_t   numValidatingPeers;
        std::string     myPublicKey;
        std::deque<std::unique_ptr<object::Peer>> validatingPeers;

        Context(std::vector<std::unique_ptr<object::Peer>>&& peers)
        {
            for (auto&& p : peers) {
                validatingPeers.push_back(std::move(p));
            }
        }
    };

    std::unique_ptr<Context> context = nullptr;

    void initializeSumeragi(const std::string& myPublicKey,
                            std::vector<std::unique_ptr<object::Peer>> peers) {
        logger::explore("sumeragi") <<  "\033[95m+==ーーーーーーーーー==+\033[0m";
        logger::explore("sumeragi") <<  "\033[95m|+-ーーーーーーーーー-+|\033[0m";
        logger::explore("sumeragi") <<  "\033[95m|| 　　　　　　　　　 ||\033[0m";
        logger::explore("sumeragi") <<  "\033[95m|| いろは合意形成機構 ||\033[0m";
        logger::explore("sumeragi") <<  "\033[95m|| 　　　\033[1mすめらぎ\033[0m\033[95m　　 ||\033[0m";
        logger::explore("sumeragi") <<  "\033[95m|| 　　　　　　　　　 ||\033[0m";
        logger::explore("sumeragi") <<  "\033[95m|+-ーーーーーーーーー-+|\033[0m";
        logger::explore("sumeragi") <<  "\033[95m+==ーーーーーーーーー==+\033[0m";
        logger::explore("sumeragi") <<  "- 起動/setup";
        logger::explore("sumeragi") <<  "- 初期設定/initialize";
        //merkle_transaction_repository::initLeaf();

        context = std::make_unique<Context>(std::move(peers));
        peers.clear();

        logger::info("sumeragi")    <<  "My key is " << config::PeerServiceConfig::getInstance().getMyIp();
        logger::info("sumeragi")    <<  "Sumeragi setted";
        logger::info("sumeragi")    <<  "set number of validatingPeer";

        context->numValidatingPeers = context->validatingPeers.size();
        context->maxFaulty = context->numValidatingPeers / (
                config::IrohaConfigManager::getInstance().getParam("numValidatingPeers", 3)
        );  // Default to approx. 1/3 of the network.

        context->proxyTailNdx = context->maxFaulty * 2 + 1;

        if (context->validatingPeers.empty()) {
            logger::error("sumeragi") << "could not find any validating peers.";
            exit(EXIT_FAILURE);
        }

        if (context->proxyTailNdx >= context->validatingPeers.size()) {
            context->proxyTailNdx = context->validatingPeers.size() - 1;
        }

        context->panicCount = 0;
        context->myPublicKey = myPublicKey;

        context->isSumeragi = context->validatingPeers.at(0)->getPublicKey() == context->myPublicKey;

        connection::receive([](const std::string& from, Event::ConsensusEvent& event) {
            logger::info("sumeragi") << "receive!";
            logger::info("sumeragi") << "received message! sig:[" << event.eventsignatures_size() << "]";
        
            // send processTransaction(event) as a task to processing pool
            // this returns std::future<void> object
            // (std::future).get() method locks processing until result of processTransaction will be available
            // but processTransaction returns void, so we don't have to call it and wait
            std::function<void()> &&task = std::bind(processTransaction, event); 
            pool.process(std::move(task)); 
        });

        logger::info("sumeragi")    <<  "initialize numValidatingPeers :"   << context->numValidatingPeers;
        logger::info("sumeragi")    <<  "initialize maxFaulty :"            << context->maxFaulty;
        logger::info("sumeragi")    <<  "initialize proxyTailNdx :"         << context->proxyTailNdx;

        logger::info("sumeragi")    <<  "initialize panicCount :"           << context->panicCount;
        logger::info("sumeragi")    <<  "initialize myPublicKey :"          << context->myPublicKey;

        //TODO: move the peer service and ordering code to another place
        //determineConsensusOrder(); // side effect is to modify validatingPeers
        logger::info("sumeragi")    <<  "initialize is sumeragi :"          << static_cast<int>(context->isSumeragi);
        logger::info("sumeragi")    <<  "initialize.....  complete!";
    }


    std::uint64_t getNextOrder() {
        return 0l;
        //return merkle_transaction_repository::getLastLeafOrder() + 1;
    }
    

    void processTransaction(Event::ConsensusEvent& event) {

        logger::info("sumeragi")    <<  "processTransaction";
        //if (!transaction_validator::isValid(event->getTx())) {
        //    return; //TODO-futurework: give bad trust rating to nodes that sent an invalid event
        //}
        logger::info("sumeragi")    <<  "valid";
        logger::info("sumeragi")    <<  "Add my signature...";

        logger::info("sumeragi")    <<  "hash:" <<  event.transaction().hash();
        logger::info("sumeragi")    <<  "pub:"  <<  config::PeerServiceConfig::getInstance().getMyPublicKey();
        logger::info("sumeragi")    <<  "pro:"  <<  config::PeerServiceConfig::getInstance().getPrivateKey();
        logger::info("sumeragi")    <<  "sog:"  <<  signature::sign(
                                                    event.transaction().hash(),
                                                    config::PeerServiceConfig::getInstance().getMyPublicKey(),
                                                    config::PeerServiceConfig::getInstance().getPrivateKey()
                                                );
        
        //detail::printIsSumeragi(context->isSumeragi);
        // Really need? blow "if statement" will be false anytime.
        detail::addSignature(event,
            config::PeerServiceConfig::getInstance().getMyPublicKey(),
            signature::sign(event.transaction().hash(),
                            config::PeerServiceConfig::getInstance().getMyPublicKey(),
                            config::PeerServiceConfig::getInstance().getPrivateKey())
        );

        if (detail::eventSignatureIsEmpty(event) && context->isSumeragi) {
            logger::info("sumeragi") << "signatures.empty() isSumragi";
            // Determine the order for processing this event
            event.set_order(getNextOrder());
            logger::info("sumeragi") << "new  order:" << event.order();
        } else if (!detail::eventSignatureIsEmpty(event)) {
            // Check if we have at least 2f+1 signatures needed for Byzantine fault tolerance
            if (detail::getNumValidSignatures(event) >= context->maxFaulty * 2 + 1) {

                logger::info("sumeragi")    <<  "Signature exists";

                logger::explore("sumeragi") <<  "0--------------------------0";
                logger::explore("sumeragi") <<  "+~~~~~~~~~~~~~~~~~~~~~~~~~~+";
                logger::explore("sumeragi") <<  "|Would you agree with this?|";
                logger::explore("sumeragi") <<  "+~~~~~~~~~~~~~~~~~~~~~~~~~~+";
                logger::explore("sumeragi") <<  "\033[93m0================================================================0\033[0m";
                logger::explore("sumeragi") <<  "\033[93m0\033[1m"  <<  event.transaction().hash()  <<  "0\033[0m";
                logger::explore("sumeragi") <<  "\033[93m0================================================================0\033[0m";

                detail::printJudge(detail::getNumValidSignatures(event), context->numValidatingPeers, context->maxFaulty * 2 + 1);

                detail::printAgree();
                // Check Merkle roots to see if match for new state
                // TODO: std::vector<std::string>>const merkleSignatures = event.merkleRootSignatures;
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
                logger::explore("sumeragi") <<  "commit";

                context->commitedCount++;

                logger::explore("sumeragi") <<  "commit count:" <<  context->commitedCount;

                merkle_transaction_repository::commit(event); //TODO: add error handling in case not saved
//
//   I want to use SerializeToString, But It has a bug??
//
//                std::string strTx;
//                event.SerializeToString(&strTx);

                const auto key = event.transaction().asset().name() + "_" + datetime::unixtime_str();
                repository::transaction::add(key, event);

                logger::debug("sumeragi")   <<  "key[" << key << "]";
                //logger::debug("sumeragi")   <<  "\033[91m+-ーーーーーーーーーーーー-+\033[0m";
                std::cout << "\033[91m+-ーーーーーーーーーーーー-+\033[0m" << std::endl;
                logger::debug("sumeragi")   <<  "tx:" << event.transaction().type();

                // I want to separate it function from sumeragi.
                if (event.transaction().type() == "Add") {

                    if (event.transaction().asset().ByteSize() != 0) {
                        logger::debug("sumeragi")   <<  "exec <Add<Asset>>";
                        convertor::decode<Add<Asset>>(event).execution();
                    } else if (event.transaction().account().ByteSize() != 0){
                        logger::debug("sumeragi")   <<  "exec <Add<Account>>";
                        convertor::decode<Add<Account>>(event).execution();
                    }

                } else if (event.transaction().type() == "Transfer"){

                    if (event.transaction().asset().ByteSize() != 0) {
                        logger::debug("sumeragi")   <<  "exec <Transfer<Asset>>";
                        convertor::decode<Transfer<Asset>>(event).execution();
                    }

                }
                // Write exec code smart contract
                // event->execution();
            } else {
                // This is a new event, so we should verify, sign, and broadcast it
                detail::addSignature(event,
                                     config::PeerServiceConfig::getInstance().getMyPublicKey(),
                                     signature::sign(event.transaction().hash(),
                                                     config::PeerServiceConfig::getInstance().getMyPublicKey(),
                                                     config::PeerServiceConfig::getInstance().getPrivateKey()).c_str());

                logger::info("sumeragi")        <<  "tail public key is "   <<  context->validatingPeers.at(context->proxyTailNdx)->getPublicKey();
                logger::info("sumeragi")        <<  "tail is "              <<  context->proxyTailNdx;
                logger::info("sumeragi")        <<  "my public key is "     <<  config::PeerServiceConfig::getInstance().getMyPublicKey();
                
                if (context->validatingPeers.at(context->proxyTailNdx)->getPublicKey() == config::PeerServiceConfig::getInstance().getMyPublicKey()) {
                    logger::info("sumeragi")    <<  "I will send event to " <<  context->validatingPeers.at(context->proxyTailNdx)->getIP();
                    connection::send(context->validatingPeers.at(context->proxyTailNdx)->getIP(), std::move(event)); // Think In Process
                } else {
                    logger::info("sumeragi")    <<  "Send All! sig:["       <<  detail::getNumValidSignatures(event) << "]";
                    connection::sendAll(std::move(event)); // TODO: Think In Process
                }

                setAwkTimer(3, [&](){
                //setAwkTimer(3000, [&](){
                    if (!merkle_transaction_repository::leafExists( event.transaction().hash())) {
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
    void panic(const Event::ConsensusEvent& event) {
        context->panicCount++; // TODO: reset this later
        auto broadcastStart = 2 * context->maxFaulty + 1 + context->maxFaulty * context->panicCount;
        auto broadcastEnd   = broadcastStart + context->maxFaulty;

        // Do some bounds checking
        if (broadcastStart > context->numValidatingPeers - 1) {
            broadcastStart = context->numValidatingPeers - 1;
        }

        if (broadcastEnd > context->numValidatingPeers - 1) {
            broadcastEnd = context->numValidatingPeers - 1;
        }
        
        logger::info("sumeragi")    <<  "broadcastEnd:"     <<  broadcastEnd;
        logger::info("sumeragi")    <<  "broadcastStart:"   <<  broadcastStart;
        // WIP issue hash event
        //connection::sendAll(event->transaction().hash()); //TODO: change this to only broadcast to peer range between broadcastStart and broadcastEnd
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
        // WIP We creat getTrustScore() function. till then circle list
        /*
        std::deque<
                std::unique_ptr<peer::Node>
        > tmp_deq;
        for(int i=1;i<context->validatingPeers.size();i++){
            tmp_deq.push_back(std::move(context->validatingPeers[i]));
        }
        tmp_deq.push_back(std::move(context->validatingPeers[0]));
        context->validatingPeers.clear();
        context->validatingPeers = std::move(tmp_deq);
        
        
        std::sort(context->validatingPeers.begin(), context->validatingPeers.end(),
              [](const std::unique_ptr<peer::Node> &lhs,
                 const std::unique_ptr<peer::Node> &rhs) {
                  return lhs->getTrustScore() > rhs->getTrustScore()
                         || (lhs->getTrustScore() == rhs->getTrustScore()
                             && lhs->getPublicKey() < rhs->getPublicKey());
              }
        );
        logger::info("sumeragi")        <<  "determineConsensusOrder sorted!";
        logger::info("sumeragi")        <<  "determineConsensusOrder myPubkey:"     <<  context->myPublicKey;

        for(const auto& peer : context->validatingPeers) {
            logger::info("sumeragi")    <<  "determineConsensusOrder PublicKey:"    <<  peer->getPublicKey();
            logger::info("sumeragi")    <<  "determineConsensusOrder ip:"           <<  peer->getIP();
        }
        */
        context->isSumeragi = context->validatingPeers.at(0)->getPublicKey() == context->myPublicKey;
    }


    void loop() {
        logger::info("sumeragi")    <<  "=+=";
        logger::info("sumeragi")    <<  "start main loop";

//        while (true) {  // 千五百秋　TODO: replace with callback linking the event repository?
//            if(!repository::event::empty()) {
//                // Determine node order
//                determineConsensusOrder();
//
//                logger::info("sumeragi")  <<  "event queue not empty";
//
//                auto events = repository::event::findAll();
//                /*
//                logger::info("sumeragi")  <<  "event's size " <<  events.size();
//                
//                // Sort the events to determine priority to process
//                std::sort(events.begin(), events.end(),
//                    [&](const auto &lhs,const auto &rhs) {
//                        return lhs->getNumValidSignatures() > rhs->getNumValidSignatures()
//                            || (context->isSumeragi && lhs->order == 0)
//                            || lhs->order < rhs->order;
//                    }
//                );
//                */
//                logger::info("sumeragi")  <<  "sorted "   <<  events.size();
//                for (auto& event : events) {
//
//                    logger::info("sumeragi")  <<  "evens order:"  <<  event.order();
//                    /*
//                    if (!transaction_validator::isValid(event)) {
//                        continue;
//                    }
//                    */
//                    // Process transaction
//                    std::thread([&event]{ processTransaction(event); }).join();
//                }
//            }
//        }
    }

};  // namespace sumeragi
