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
#include <atomic>
#include <cmath>
#include <iterator>
#include <map>
#include <string>
#include <thread>

#include <thread_pool.hpp>

#include <crypto/hash.hpp>
#include <crypto/signature.hpp>
#include <repository/consensus/merkle_transaction_repository.hpp>
#include <util/logger.hpp>

#include <consensus/connection/connection.hpp>
#include <infra/config/peer_service_with_json.hpp>
#include <repository/transaction_repository.hpp>
#include <service/peer_service.hpp>
#include <service/peer_service.hpp>
#include <validation/transaction_validator.hpp>

#include <infra/config/iroha_config_with_json.hpp>
#include <service/executor.hpp>

/**
* |ーーー|　|ーーー|　|ーーー|　|ーーー|
* |　ス　|ー|　メ　|ー|　ラ　|ー|　ギ　|
* |ーーー|　|ーーー|　|ーーー|　|ーーー|
*
* A chain-based byzantine fault tolerant consensus algorithm, based in large
* part on BChain:
*
* Duan, S., Meling, H., Peisert, S., & Zhang, H. (2014). Bchain: Byzantine
* replication with
* high throughput and embedded reconfiguration. In International Conference on
* Principles of
* Distributed Systems (pp. 91-106). Springer.
*/
namespace sumeragi {

using Api::ConsensusEvent;
using Api::Signature;
using Api::Transaction;

std::map<std::string, std::string> txCache;

static ThreadPool pool(ThreadPoolOptions{
    .threads_count =
        config::IrohaConfigManager::getInstance().getConcurrency(0),
    .worker_queue_size =
        config::IrohaConfigManager::getInstance().getPoolWorkerQueueSize(1024),
});

namespace detail {

std::string hash(const Transaction &tx) {
  return hash::sha3_256_hex(tx.SerializeAsString());
};

void addSignature(ConsensusEvent &event, const std::string &publicKey,
                  const std::string &signature) {
  Signature sig;
  sig.set_signature(signature);
  sig.set_publickey(publicKey);
  event.add_eventsignatures()->CopyFrom(sig);
}

bool eventSignatureIsEmpty(const ConsensusEvent &event) {
  return event.eventsignatures_size() == 0;
}

void printIsSumeragi(bool isSumeragi) {
  if (isSumeragi) {
    logger::explore("sumeragi") << "===+==========+===";
    logger::explore("sumeragi") << "   |  |=+=|   |";
    logger::explore("sumeragi") << "  -+----------+-";
    logger::explore("sumeragi") << "   |          |";
    logger::explore("sumeragi") << "   |  I  am   |";
    logger::explore("sumeragi") << "   | Sumeragi |";
    logger::explore("sumeragi") << "   |          |";
    logger::explore("sumeragi") << "   A          A";
  } else {
    logger::explore("sumeragi") << "   /\\         /\\";
    logger::explore("sumcderagi") << "   ||  I  am  ||";
    logger::explore("sumeragi") << "   ||   peer  ||";
    logger::explore("sumeragi") << "   ||         ||";
    logger::explore("sumeragi") << "   AA         AA";
  }
}

void printJudge(int numValidSignatures, int numValidationPeer, int faulty) {
  std::stringstream resLine[5];
  for (int i = 0; i < numValidationPeer; i++) {
    if (i < numValidSignatures) {
      resLine[0] << "\033[1m\033[92m+-ー-+\033[0m";
      resLine[1] << "\033[1m\033[92m| 　 |\033[0m";
      resLine[2] << "\033[1m\033[92m|-承-|\033[0m";
      resLine[3] << "\033[1m\033[92m| 　 |\033[0m";
      resLine[4] << "\033[1m\033[92m+-＝-+\033[0m";
    } else {
      resLine[0] << "\033[91m+-ー-+\033[0m";
      resLine[1] << "\033[91m| 　 |\033[0m";
      resLine[2] << "\033[91m| 否 |\033[0m";
      resLine[3] << "\033[91m| 　 |\033[0m";
      resLine[4] << "\033[91m+-＝-+\033[0m";
    }
  }
  for (int i = 0; i < 5; ++i)
    logger::explore("sumeragi") << resLine[i].str();

  std::string line;
  for (int i = 0; i < numValidationPeer; i++)
    line += "==＝==";
  logger::explore("sumeragi") << line;

  logger::explore("sumeragi") << "numValidSignatures:" << numValidSignatures
                              << " faulty:" << faulty;
}

void printAgree() {
  logger::explore("sumeragi") << "\033[1m\033[92m+==ーー==+\033[0m";
  logger::explore("sumeragi") << "\033[1m\033[92m|+-ーー-+|\033[0m";
  logger::explore("sumeragi") << "\033[1m\033[92m|| 承認 ||\033[0m";
  logger::explore("sumeragi") << "\033[1m\033[92m|+-ーー-+|\033[0m";
  logger::explore("sumeragi") << "\033[1m\033[92m+==ーー==+\033[0m";
}

void printReject() {
  logger::explore("sumeragi") << "\033[91m+==ーー==+\033[0m";
  logger::explore("sumeragi") << "\033[91m|+-ーー-+|\033[0m";
  logger::explore("sumeragi") << "\033[91m|| 否認 ||\033[0m";
  logger::explore("sumeragi") << "\033[91m|+-ーー-+|\033[0m";
  logger::explore("sumeragi") << "\033[91m+==ーー==+\033[0m";
}
} // namespace detail

struct Context {
  bool isSumeragi;         // am I the leader or am I not?
  std::uint64_t maxFaulty; // f
  std::uint64_t proxyTailNdx;
  std::int32_t panicCount;
  std::int64_t commitedCount = 0;
  std::uint64_t numValidatingPeers;
  std::string myPublicKey;
  peer::Nodes validatingPeers;

  Context() { update(); }

  void update() {
    logger::debug("sumeragi") << "Context update!";
    validatingPeers = ::peer::service::getPeerList();

    numValidatingPeers = validatingPeers.size();
    // maxFaulty = Default to approx. 1/3 of the network.
    maxFaulty = ::peer::service::getMaxFaulty();
    proxyTailNdx = this->maxFaulty * 2 + 1;

    if (validatingPeers.empty()) {
      logger::error("sumeragi") << "could not find any validating peers.";
      exit(EXIT_FAILURE);
    }

    if (proxyTailNdx >= validatingPeers.size()) {
      proxyTailNdx = validatingPeers.size() - 1;
    }

    panicCount = 0;
    myPublicKey = ::peer::myself::getPublicKey();

    isSumeragi = validatingPeers.at(0)->publicKey == myPublicKey;
  }
};

std::unique_ptr<Context> context = nullptr;

void initializeSumeragi() {
  logger::explore("sumeragi") << "\033[95m+==ーーーーーーーーー==+\033[0m";
  logger::explore("sumeragi") << "\033[95m|+-ーーーーーーーーー-+|\033[0m";
  logger::explore("sumeragi") << "\033[95m|| 　　　　　　　　　 ||\033[0m";
  logger::explore("sumeragi") << "\033[95m|| いろは合意形成機構 ||\033[0m";
  logger::explore("sumeragi")
      << "\033[95m|| 　　　\033[1mすめらぎ\033[0m\033[95m　　 ||\033[0m";
  logger::explore("sumeragi") << "\033[95m|| 　　　　　　　　　 ||\033[0m";
  logger::explore("sumeragi") << "\033[95m|+-ーーーーーーーーー-+|\033[0m";
  logger::explore("sumeragi") << "\033[95m+==ーーーーーーーーー==+\033[0m";
  logger::explore("sumeragi") << "- 起動/setup";
  logger::explore("sumeragi") << "- 初期設定/initialize";
  // merkle_transaction_repository::initLeaf();

  logger::info("sumeragi") << "My key is " << ::peer::myself::getIp();
  logger::info("sumeragi") << "Sumeragi setted";
  logger::info("sumeragi") << "set number of validatingPeer";

  context = std::make_unique<Context>();

  connection::iroha::Sumeragi::Torii::receive(
      [](const std::string &from, Transaction &transaction) {
        logger::info("sumeragi") << "receive! Torii";
        ConsensusEvent event;
        event.set_status("uncommit");
        event.mutable_transaction()->CopyFrom(transaction);
        context->update();
        // send processTransaction(event) as a task to processing pool
        // this returns std::future<void> object
        // (std::future).get() method locks processing until result of
        // processTransaction will be available
        // but processTransaction returns void, so we don't have to call it and
        // wait
        std::function<void()> &&task = std::bind(processTransaction, event);
        pool.process(std::move(task));
      });

  connection::iroha::Sumeragi::Verify::receive([](const std::string &from,
                                                  ConsensusEvent &event) {
    logger::info("sumeragi") << "receive!";
    logger::info("sumeragi") << "received message! sig:["
                             << event.eventsignatures_size() << "]";
    logger::info("sumeragi") << "received message! status:[" << event.status()
                             << "]";
    if (event.status() == "commited") {
      if (txCache.find(detail::hash(event.transaction())) == txCache.end()) {
        txCache[detail::hash(event.transaction())] = "commited";
        repository::transaction::add(detail::hash(event.transaction()),
                                     event.transaction());
        executor::execute(event.transaction());
      }
    } else {
      // send processTransaction(event) as a task to processing pool
      // this returns std::future<void> object
      // (std::future).get() method locks processing until result of
      // processTransaction will be available
      // but processTransaction returns void, so we don't have to call it and
      // wait
      std::function<void()> &&task = std::bind(processTransaction, event);
      pool.process(std::move(task));
    }
  });

  logger::info("sumeragi") << "initialize numValidatingPeers :"
                           << context->numValidatingPeers;
  logger::info("sumeragi") << "initialize maxFaulty :" << context->maxFaulty;
  logger::info("sumeragi") << "initialize proxyTailNdx :"
                           << context->proxyTailNdx;

  logger::info("sumeragi") << "initialize panicCount :" << context->panicCount;
  logger::info("sumeragi") << "initialize myPublicKey :"
                           << context->myPublicKey;

  // TODO: move the peer service and ordering code to another place
  // determineConsensusOrder(); // side effect is to modify validatingPeers
  logger::info("sumeragi") << "initialize is sumeragi :"
                           << static_cast<int>(context->isSumeragi);
  logger::info("sumeragi") << "initialize.....  complete!";
}

std::uint64_t getNextOrder() {
  return 0l;
  // return merkle_transaction_repository::getLastLeafOrder() + 1;
}

void processTransaction(ConsensusEvent &event) {

  logger::info("sumeragi") << "processTransaction";
  // if (!transaction_validator::isValid(event->getTx())) {
  //    return; //TODO-futurework: give bad trust rating to nodes that sent an
  //    invalid event
  //}
  logger::info("sumeragi") << "valid";
  logger::info("sumeragi") << "Add my signature...";

  logger::info("sumeragi") << "hash:" << detail::hash(event.transaction());
  logger::info("sumeragi") << "pub: " << ::peer::myself::getPublicKey();
  logger::info("sumeragi") << "priv:" << ::peer::myself::getPrivateKey();
  logger::info("sumeragi") << "sig: "
                           << signature::sign(detail::hash(event.transaction()),
                                              ::peer::myself::getPublicKey(),
                                              ::peer::myself::getPrivateKey());

  // detail::printIsSumeragi(context->isSumeragi);
  // Really need? blow "if statement" will be false anytime.
  detail::addSignature(event, ::peer::myself::getPublicKey(),
                       signature::sign(detail::hash(event.transaction()),
                                       ::peer::myself::getPublicKey(),
                                       ::peer::myself::getPrivateKey()));

  if (detail::eventSignatureIsEmpty(event) && context->isSumeragi) {
    logger::info("sumeragi") << "signatures.empty() isSumragi";
    // Determine the order for processing this event
    event.set_order(getNextOrder()); // TODO getNexOrder is always return 0l;
    logger::info("sumeragi") << "new  order:" << event.order();
  } else if (!detail::eventSignatureIsEmpty(event)) {
    // Check if we have at least 2f+1 signatures needed for Byzantine fault
    // tolerance
    if (transaction_validator::countValidSignatures(event) >=
        context->maxFaulty * 2 + 1) {

      logger::info("sumeragi") << "Signature exists";

      logger::explore("sumeragi") << "0--------------------------0";
      logger::explore("sumeragi") << "+~~~~~~~~~~~~~~~~~~~~~~~~~~+";
      logger::explore("sumeragi") << "|Would you agree with this?|";
      logger::explore("sumeragi") << "+~~~~~~~~~~~~~~~~~~~~~~~~~~+";
      logger::explore("sumeragi") << "\033[93m0================================"
                                     "================================0\033[0m";
      logger::explore("sumeragi") << "\033[93m0\033[1m"
                                  << detail::hash(event.transaction())
                                  << "0\033[0m";
      logger::explore("sumeragi") << "\033[93m0================================"
                                     "================================0\033[0m";

      detail::printJudge(transaction_validator::countValidSignatures(event),
                         context->numValidatingPeers,
                         context->maxFaulty * 2 + 1);

      detail::printAgree();
      // Check Merkle roots to see if match for new state
      // TODO: std::vector<std::string>>const merkleSignatures =
      // event.merkleRootSignatures;
      // Try applying transaction locally and compute the merkle root

      // Commit locally
      logger::explore("sumeragi") << "commit";

      context->commitedCount++;

      logger::explore("sumeragi") << "commit count:" << context->commitedCount;

      merkle_transaction_repository::commit(
          event); // TODO: add error handling in case not saved
      event.set_status("commited");
      connection::iroha::Sumeragi::Verify::sendAll(std::move(event));

    } else {
      // This is a new event, so we should verify, sign, and broadcast it
      detail::addSignature(event, ::peer::myself::getPublicKey(),
                           signature::sign(detail::hash(event.transaction()),
                                           ::peer::myself::getPublicKey(),
                                           ::peer::myself::getPrivateKey())
                               .c_str());

      logger::info("sumeragi")
          << "tail public key is "
          << context->validatingPeers.at(context->proxyTailNdx)->publicKey;
      logger::info("sumeragi") << "tail is " << context->proxyTailNdx;
      logger::info("sumeragi") << "my public key is "
                               << ::peer::myself::getPublicKey();

      if (context->validatingPeers.at(context->proxyTailNdx)->publicKey ==
          ::peer::myself::getPublicKey()) {
        logger::info("sumeragi")
            << "I will send event to "
            << context->validatingPeers.at(context->proxyTailNdx)->ip;
        connection::iroha::Sumeragi::Verify::send(
            context->validatingPeers.at(context->proxyTailNdx)->ip,
            std::move(event)); // Think In Process
      } else {
        logger::info("sumeragi")
            << "Send All! sig:["
            << transaction_validator::countValidSignatures(event) << "]";
        connection::iroha::Sumeragi::Verify::sendAll(
            std::move(event)); // TODO: Think In Process
      }

      setAwkTimer(3000, [&]() {
        if (!merkle_transaction_repository::leafExists(
                detail::hash(event.transaction()))) {
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
* the set of considered validators, A, is expanded by f (e.g., by 1 in the
* example below):
*  ________________________    __________
* /           A            \  /    B     \
* |---|  |---|  |---|  |---|  |---|  |---|
* | 0 |--| 1 |--| 2 |--| 3 |--| 4 |--| 5 |
* |---|  |---|  |---|  |---|  |---|  |---|.
*/
void panic(const ConsensusEvent &event) {
  context->panicCount++; // TODO: reset this later
  auto broadcastStart =
      2 * context->maxFaulty + 1 + context->maxFaulty * context->panicCount;
  auto broadcastEnd = broadcastStart + context->maxFaulty;

  // Do some bounds checking
  if (broadcastStart > context->numValidatingPeers - 1) {
    broadcastStart = context->numValidatingPeers - 1;
  }

  if (broadcastEnd > context->numValidatingPeers - 1) {
    broadcastEnd = context->numValidatingPeers - 1;
  }

  logger::info("sumeragi") << "broadcastEnd:" << broadcastEnd;
  logger::info("sumeragi") << "broadcastStart:" << broadcastStart;
  // WIP issue hash event
  // connection::sendAll(event->transaction().hash()); //TODO: change this to
  // only broadcast to peer range between broadcastStart and broadcastEnd
}

void setAwkTimer(int const sleepMillisecs,
                 std::function<void(void)> const action) {
  //    std::thread([action, sleepMillisecs]() {
  //        std::this_thread::sleep_for(std::chrono::milliseconds(sleepMillisecs));
  //        action();
  //    }).join();
  std::this_thread::sleep_for(std::chrono::milliseconds(sleepMillisecs));
  action();
}

/**
 * The consensus order is based primarily on the trust scores. If two trust
 * scores
 * are the same, then the order (ascending) of the public keys for the servers
 * are used.
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
            return lhs->trustScore > rhs->trustScore
                   || (lhs->trustScore == rhs->trustScore
                       && lhs->publicKey < rhs->publicKey);
        }
  );
  logger::info("sumeragi")        <<  "determineConsensusOrder sorted!";
  logger::info("sumeragi")        <<  "determineConsensusOrder myPubkey:"     <<
  context->myPublicKey;

  for(const auto& peer : context->validatingPeers) {
      logger::info("sumeragi")    <<  "determineConsensusOrder PublicKey:"    <<
  peer->publicKey;
      logger::info("sumeragi")    <<  "determineConsensusOrder ip:"           <<
  peer->ip;
  }
  */
  context->isSumeragi =
      context->validatingPeers.at(0)->publicKey == context->myPublicKey;
    }

};  // namespace sumeragi
